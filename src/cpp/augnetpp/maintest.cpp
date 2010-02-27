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

#include "augnetpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    struct callbacks {
        aug_result
        base64cb(const char* buf, size_t len)
        {
            return 0;
        }
    };

    struct httptest {
        aug_result
        httprequest_(const char* value) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        httpfield_(const char* name, const char* value) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        httpcsize_(unsigned csize) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        httpcdata_(const void* cdata, unsigned csize) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        httpend_(aug_bool commit) AUG_NOTHROW
        {
            return 0;
        }
    };

    struct martest {
        aug_result
        delmar_(const char* request) AUG_NOTHROW
        {
            return 0;
        }
        aug_mar_*
        getmar_(const char* request) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        putmar_(const char* request, aug_mar_* mar) AUG_NOTHROW
        {
            return 0;
        }
    };
}

int
main(int argc, char* argv[])
{
    try {
        autotlx();

        callbacks cbs;
        mpoolptr mp(getmpool(aug_tlx));
        base64 b64(mp, AUG_ENCODE64, cbs);

        scoped_httphandler_wrapper<httptest> x;
        httpparser hparser(mp, x, 1024);

        scoped_marstore_wrapper<martest> y;
        marparser mparser(mp, y, 1024);

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
