/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/

#include "augnetpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    struct callbacks : mpool_ops {
        aug_result
        base64cb(const char* buf, size_t len)
        {
            return AUG_SUCCESS;
        }
    };

    struct httptest : mpool_ops {
        aug_result
        httpinitial_(const char* value) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
        aug_result
        httpfield_(const char* name, const char* value) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
        aug_result
        httpcsize_(unsigned csize) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
        aug_result
        httpcdata_(const void* cdata, unsigned csize) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
        aug_result
        httpend_(aug_bool commit) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
    };

    struct martest : mpool_ops {
        aug_result
        delmar_(const char* initial) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
        aug_mar_*
        getmar_(const char* initial) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        putmar_(const char* initial, aug_mar_* mar) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
    };
}

int
main(int argc, char* argv[])
{
    try {
        autodltlx();

        callbacks cbs;
        mpoolptr mp(getmpool(aug_tlx));
        base64 b64(mp, AUG_ENCODE64, cbs);

        scoped_httphandler_wrapper<httptest> x;
        httpparser hparser(mp, x, 1024);

        scoped_marpool_wrapper<martest> y;
        marparser mparser(mp, y, 1024);

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
