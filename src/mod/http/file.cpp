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

#include "exception.hpp"

#include "augmodpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augmarpp.hpp"
#include "augsyspp.hpp"

#include "augtypes.h"

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

    class nodes : public mpool_ops {
        vector<string>* xs_;
    public:
        explicit
        nodes(vector<string>& xs)
            : xs_(&xs)
        {
        }
        void
        operator ()(string& x)
        {
            if (x == "..") {
                if (xs_->empty())
                    throw http_error(403, "Forbidden");
                xs_->pop_back();
            } else if (!x.empty() && x != ".")
                xs_->push_back(x);
        }
    };

    class filecontent : public ref_base, public mpool_ops {
        blob<filecontent> blob_;
        autofd sfd_;
        mmap mmap_;
        ~filecontent() AUG_NOTHROW
        {
        }
        filecontent(mpoolref mpool, const char* path)
            : sfd_(aug::open(path, O_RDONLY)),
              mmap_(mpool, sfd_, 0, 0, AUG_MMAPRD)
        {
            blob_.reset(this);
        }
    public:
        smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_blob>(id))
                return object_retain<aug_object>(blob_);
            return null;
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
        static smartob<aug_blob>
        create(const char* path)
        {
            filecontent* ptr = new (tlx) filecontent(getmpool(aug_tlx), path);
            return object_attach<aug_blob>(ptr->blob_);
        }
    };

    bool
    lstat(const char* path, struct _stat& sb)
    {
        // lstat() is used so that the link, rather than the file it
        // references, is stat()-ed.

		if (-1 == ::lstat(path, &sb)) {
            if (ENOENT != errno)
                throw posix_error(__FILE__, __LINE__, errno);
            return false;
        }
        return true;
    }
}

blobptr
aug::getfile(const char* path)
{
    return filecontent::create(path);
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
aug::joinpath(const char* root, const vector<string>& nodes)
{
    string path(root);
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

vector<string>
aug::splitpath(const string& path)
{
    vector<string> v;
    splitn(path.begin(), path.end(), '/', nodes(v));
    return v;
}

