/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutilpp/string.hpp"
#include "augutil/pwd.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#if !defined(_WIN32)
# include <unistd.h>
#else  // _WIN32
# include <io.h>
#endif // _WIN32

#if !defined(R_OK)
# define R_OK 4
#endif // !R_OK

using namespace aug;
using namespace std;

// A GPL alternative to the Apache htdigest program.

// $ htdigest
// Usage: htdigest [-c] passwordfile realm username
// The -c flag creates a new file.

// $ htdigest -c passwd.txt home marayl
// New password:
// Re-type new password:
// Adding password for marayl in realm home.

// $ htdigest.exe passwd.txt home marayl
// New password:
// Re-type new password:
// Changing password for user marayl in realm home.

// Other strings from strings program:

// New password:
// Re-type new password:
// They don't match, sorry.
// Usage: htdigest [-c] passwordfile realm username
// The -c flag creates a new file.
// Interrupted.
// Could not open passwd file %s for writing.
// Adding password for %s in realm %s.
// Could not open temp file.
// Could not open passwd file %s for reading.
// Use -c option to create new one.
// Changing password for user %s in realm %s
// Adding user %s in realm %s

namespace {

    void
    usage()
    {
        cerr << "Usage: htdigest [-c] passwordfile realm username\n"
            "The -c flag creates a new file.\n";
    }

    class error : public runtime_error {
    public:
        explicit
        error(const string& s)
            : runtime_error(s)
        {
        }
        error(const string& s1, const string& s2)
            : runtime_error(s1 + ": " + s2)
        {
        }
    };

    string
    mkswap(string path)
    {
        path += ".XXXXXX";

        vector<char> v;
        copy(path.begin(), path.end(), back_inserter(v));
        v.push_back('\0');

        if (!mktemp(&v[0]))
            throw error("mktemp() failed", path);

        return &v[0];
    }

    class swapfile {
        const string master_;
        const string swap_;
        ofstream os_;
        bool rollback_;
    public:
        ~swapfile()
        {
            if (rollback_) {
                // Ensure closed before unlinking.
                if (os_.is_open())
                    os_.close();
                unlink(swap_.c_str());
            }
        }
        explicit
        swapfile(const string& master)
            : master_(master),
              swap_(mkswap(master)),
              os_(swap_.c_str()),
              rollback_(true)
        {
            if (!os_)
                throw error("open() failed", swap_);
        }
        void
        commit()
        {
            os_.close();
#if defined(_WIN32)
            // Cannot rename to existing target on Windows.
            unlink(master_.c_str());
#endif // _WIN32
            if (rename(swap_.c_str(), master_.c_str()) < 0)
                throw error("rename() failed", strerror(errno));
            rollback_ = false;
        }
        ostream&
        os()
        {
            return os_;
        }
    };

    string
    todigest(const char* user, const char* realm, const char* pass)
    {
        aug_md5base64_t base64;
        if (!aug_digestpass(user, realm, pass, base64))
            throw error("aug_digestpass() failed");
        return base64;
    }

    void
    print(ostream& os, const char* user, const char* realm,
          const string& digest)
    {
        os << user << ':' << realm << ':' << digest << endl;
    }

    bool
    update(istream& is, ostream& os, const char* user, const char* realm,
           const string& digest)
    {
        bool updated(false);

        string raw;
        while (getline(is, raw)) {

            const string line(trimcopy(raw));

            // If not blank or comment.

            if (!line.empty() && '#' != line[0]) {

                vector<string> toks(splitn(line.begin(), line.end(), ':'));

                // And matches user and realm.

                if (3 == toks.size() && toks[0] == user && toks[1] == realm) {

                    // Then update.

                    cout << "Changing password for " << user << " in realm "
                         << realm << ".\n";
                    print(os, user, realm, digest);
                    updated = true;
                    continue;
                }
            }

            // Preserve original formatting on unchanged lines.

            os << raw << endl;
        }

        return updated;
    }

    bool
    update(const char* in, ostream& os, const char* user, const char* realm,
           const string& digest)
    {
        ifstream is(in);
        return update(is, os, user, realm, digest);
    }
}

int
main(int argc, char* argv[])
{
    // TODO: add signal handler.

    // Tie iostreams and stdio.

    ios::sync_with_stdio();

    try {

        bool create;
        switch (argc) {
        case 4:
            // No -c option.
            create = false;
            break;
        case 5:
            if (0 != strcmp(argv[1], "-c")) {
                usage();
                cerr << "Invalid argument; expecting '-c'.\n";
                return 1;
            }
            // Consume -c option.
            --argc;
            ++argv;
            create = true;
            break;
        default:
            usage();
            cerr << "Invalid number of arguments.\n";
            return 1;
        }

        // Required arguments.

        const char* const path(argv[1]);
        const char* const realm(argv[2]);
        const char* const user(argv[3]);

        // If not creating, then ensure existing file is readable.

        if (!create && access(path, R_OK) < 0) {
            cerr << "Could not open passwd file " << path
                 << " for reading.\n";
            return 1;
        }

        // Get password.

        aug_pwd_t pwd1, pwd2;
        if (!aug_getpass("New password:", pwd1, sizeof(pwd1))
            || !aug_getpass("Re-type new password:", pwd2, sizeof(pwd2)))
            return 1;

        // Verify password.

        if (0 != strcmp(pwd1, pwd2)) {
            cerr << "They don't match, sorry.\n";
            return 1;
        }

        // Password digest.

        const string digest(todigest(user, realm, pwd1));

        // Done with passwords.

        memset(pwd1, 0, sizeof(pwd1));
        memset(pwd2, 0, sizeof(pwd2));

        // Write modifications to swap file.

        swapfile sf(path);
        if (create || !update(path, sf.os(), user, realm, digest)) {

            cout << "Adding password for " << user << " in realm "
                 << realm << ".\n";
            print(sf.os(), user, realm, digest);
        }

        // Replace master with swap.

        sf.commit();
        return 0;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 1;
}
