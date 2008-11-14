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

    struct callbacks {
        aug_result
        base64cb(const char* buf, size_t len)
        {
            return AUG_SUCCESS;
        }
    };

    struct httptest {
        aug_result
        initial(const char* value)
        {
            return AUG_SUCCESS;
        }
        aug_result
        field(const char* name, const char* value)
        {
            return AUG_SUCCESS;
        }
        aug_result
        csize(unsigned csize)
        {
            return AUG_SUCCESS;
        }
        aug_result
        cdata(const void* cdata, unsigned csize)
        {
            return AUG_SUCCESS;
        }
        aug_result
        end(bool commit)
        {
            return AUG_SUCCESS;
        }
    };

    struct martest {
        aug_result
        delmar_(const char* initial) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
        struct aug_mar_*
        getmar_(const char* initial) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        putmar_(const char* initial, struct aug_mar_* mar) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
    };
}

int
main(int argc, char* argv[])
{
    try {
        autobasictlx();

        callbacks cbs;
        mpoolptr mp(getmpool(aug_tlx));
        base64 b64(mp, AUG_ENCODE64, cbs);

        httptest x;
        httpparser hparser(mp, 1024, x);

        scoped_marpool<martest> y;
        marparser mparser(mp, y, 1024);

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
