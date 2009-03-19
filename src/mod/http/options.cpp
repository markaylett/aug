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

#include "augutilpp/file.hpp"
#include "augutilpp/path.hpp"

#include "augmodpp.hpp"

#include "augtypes.h"

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    aug_result
    mimecb(void* arg, const char* name, const char* value)
    {
        map<string, string>& mimetypes
            (*static_cast<map<string, string>*>(arg));
        mimetypes[name] = value;
        return AUG_SUCCESS;
    }

    void
    setmimetypes(map<string, string>& mimetypes)
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
            string path(makepath(mod::getenv("rundir"), s));
            readconf(path.c_str(), confcb<mimecb>, null);
        }
    }
}

void
options::load()
{
    map<string, string> mimetypes;
    setmimetypes(mimetypes);

    // Commit.

    mimetypes_.swap(mimetypes);
}

string
options::mimetype(const string& path)
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
