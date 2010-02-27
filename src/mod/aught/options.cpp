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
#define MOD_BUILD
#include "options.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutilpp/file.hpp"
#include "augutilpp/path.hpp"
#include "augutilpp/string.hpp"

#include "augmodpp.hpp"

#include <fstream>
#include <iterator>
#include <sstream>

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    void
    loadcss(string& css)
    {
        const char* s(mod::getenv("session.http.css"));
        if (s) {
            char path[AUG_PATH_MAX + 1];
            abspath(mod::getenv("rundir"), s, path, sizeof(path));
            ifstream fs(path);
            stringstream ss;
            ss << fs.rdbuf();
            css = ss.str();
        }
    }

    aug_result
    mimecb(const char* name, const char* value, void* arg)
    {
        map<string, string>& mimetypes
            (*static_cast<map<string, string>*>(arg));
        mimetypes[name] = value;
        return 0;
    }

    void
    loadmimetypes(map<string, string>& mimetypes)
    {
        // Add some basic types.

        mimetypes["css"] = "text/css";
        mimetypes["gif"] = "image/gif";
        mimetypes["html"] = "text/html";
        mimetypes["htm"] = "text/html";
        mimetypes["jpeg"] = "image/jpeg";
        mimetypes["jpg"] = "image/jpeg";
        mimetypes["js"] = "application/x-javascript";
        mimetypes["png"] = "image/png";
        mimetypes["tif"] = "image/tiff";
        mimetypes["tiff"] = "image/tiff";
        mimetypes["txt"] = "text/plain";
        mimetypes["xml"] = "text/xml";

        const char* s(mod::getenv("session.http.mimetypes"));
        if (s) {
            string path(abspath(mod::getenv("rundir"), s));
            readconf(path.c_str(), confcb<mimecb>, &mimetypes);
        }
    }

    void
    loadpasswd(map<pair<string, string>, string>& passwd)
    {
        const char* s(mod::getenv("session.http.passwd"));
        if (s) {

            char path[AUG_PATH_MAX + 1];
            abspath(mod::getenv("rundir"), s, path, sizeof(path));

            ifstream fs(path);
            string line;
            while (getline(fs, line)) {

                trim(line);
                if (line.empty() || '#' == line[0])
                    continue;

                // username:realm:digest

                vector<string> toks(splitn(line.begin(), line.end(), ':'));
                if (3 == toks.size())
                    passwd.insert(make_pair(make_pair(toks[0], toks[1]),
                                            toks[2]));
            }
        }
    }

    void
    loadservices(outer_base& outer)
    {
        const char* s(mod::getenv("session.http.services"));
        if (s) {
            istringstream is(s);
            copy(istream_iterator<string>(is), istream_iterator<string>(),
                 outer_inserter(outer));
        }
    }
}

void
options::load()
{
    string css;
    map<string, string> mimetypes;
    map<pair<string, string>, string> passwd;
    rootptr services(createroot());

    loadcss(css);
    loadmimetypes(mimetypes);
    loadpasswd(passwd);
    loadservices(*services);

    // Commit.

    css_.swap(css);
    mimetypes_.swap(mimetypes);
    passwd_.swap(passwd);
    services_.swap(services);
}

string
options::mimetype(const string& path) const
{
    string type("text/plain");

    // Resolve mimetype from file extension.

    string::size_type pos(path.find_last_of('.'));
    if (pos != string::npos) {
        string ext(path.substr(pos + 1));
        map<string, string>::const_iterator it(mimetypes_.find(ext));
        if (it != mimetypes_.end())
            type = it->second;
    }

    return type;
}

bool
options::passwd(const string& user, const string& realm, string& digest)
{
    map<pair<string, string>, string>
        ::const_iterator it(passwd_.find(make_pair(user, realm)));
    if (it == passwd_.end())
        return false;

    digest = it->second;
    return true;
}
