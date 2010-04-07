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
#include "file.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "exception.hpp"

#include "augutilpp/path.hpp"

#include "augsyspp/mmap.hpp"
#include "augsyspp/unistd.hpp"
#include "augsyspp/smartfd.hpp"
#include "augctxpp/mpool.hpp"

#include "augmodpp.hpp"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#if !defined(_WIN32)
# define _stat stat
# define _S_ISDIR S_ISDIR
# define _S_ISREG S_ISREG
# define _S_IREAD S_IROTH
#else // _WIN32
# define lstat _stat
# if !defined(_S_ISDIR)
#  define _S_ISDIR(mode) (((mode) & _S_IFDIR) == _S_IFDIR)
# endif // !_S_ISDIR
# if !defined(_S_ISREG)
#  define _S_ISREG(mode) (((mode) & _S_IFREG) == _S_IFREG)
# endif // !_S_ISREG
#endif // _WIN32

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    class filecontent : public blob_base<filecontent>, public mpool_ops {
        const string type_;
        autofd sfd_;
        mmap mmap_;
        filecontent(mpoolref mpool, const string& type, const char* path)
            : type_(type),
              sfd_(aug::open(path, O_RDONLY)),
              mmap_(mpool, sfd_, 0, 0, AUG_MMAPRD)
        {
        }
    public:
        ~filecontent() AUG_NOTHROW
        {
        }
        const char*
        getblobtype_()
        {
            return type_.c_str();
        }
        const void*
        getblobdata_(size_t& size) AUG_NOTHROW
        {
            size = mmap_.len();
            return mmap_.addr();
        }
        size_t
        getblobsize_() AUG_NOTHROW
        {
            return mmap_.len();
        }
        static blobptr
        create(const string& type, const char* path)
        {
            filecontent* ptr
                = new (tlx) filecontent(getmpool(aug_tlx), type, path);
            return attach(ptr);
        }
    };

    bool
    lstat(const char* path, struct _stat& sb)
    {
        // lstat() is used so that the link, rather than the file it
        // references, is stat()-ed.

		if (::lstat(path, &sb) < 0) {
            if (ENOENT != errno)
                throw posix_error(__FILE__, __LINE__, errno);
            return false;
        }
        return true;
    }
}

blobptr
aug::getfile(const string& type, const char* path)
{
    return filecontent::create(type, path);
}

string
aug::jointype(const vector<string>& nodes)
{
    string type;
    vector<string>::const_iterator it(nodes.begin()), end(nodes.end());
    for (; it != end; ++it) {
        if (!type.empty())
            type += '/';
        type += *it;
    }
    return type;
}

string
aug::joinpath(const vector<string>& nodes)
{
    const char* s = mod::getenv("htdocs", "htdocs");
    string path(abspath(mod::getenv("rundir"), s));
    struct _stat sb;

    vector<string>::const_iterator it(nodes.begin()), end(nodes.end());
    for (; it != end; ++it) {
        path += '/';
        path += *it;

        if (!lstat(path.c_str(), sb))
            throw http_error(404, "Not Found");

        // Do not follow symbolic links.

        if (!_S_ISDIR(sb.st_mode) && !_S_ISREG(sb.st_mode))
            throw http_error(403, "Forbidden");
    }

    // Use index.html if directory.

    if (nodes.empty() || _S_ISDIR(sb.st_mode)) {
        path += "/index.html";
        if (!lstat(path.c_str(), sb))
            throw http_error(404, "Not Found");
    }

    // Must be regular, world-readable file.

    if (!_S_ISREG(sb.st_mode) || !(sb.st_mode & _S_IREAD))
        throw http_error(403, "Forbidden");

    return path;
}
