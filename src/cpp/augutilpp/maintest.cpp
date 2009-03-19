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

#include "augutilpp/object.hpp"

#include <iostream>
#include <stdexcept>

using namespace aug;
using namespace std;

namespace {
    typedef logic_error error;

    void
    test(blobref blob, const string& s)
    {
        size_t size;
        const void* data(getblobdata(blob, size));

        if (size != s.size())
            throw error("size mismatch");

        if (string(static_cast<const char*>(data), size) != s)
            throw error("data mismatch");
    }
}

int
main(int argc, char* argv[])
{
    try {

        const string s("some test data");

        smartob<aug_blob> smart(blob_wrapper<sblob>::create(s));
        test(smart, s);

        if (null == smart)
            throw error("bad null equality");

        smart = null;
        if (null != smart)
            throw error("bad null inequality");

        scoped_blob_wrapper<sblob> scoped(s);
        test(scoped, s);

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
