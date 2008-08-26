/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
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
        void
        base64cb(const char* buf, size_t len)
        {
        }
    };

    struct httphandler {
        void
        initial(const char* value)
        {
        }
        void
        field(const char* name, const char* value)
        {
        }
        void
        csize(unsigned csize)
        {
        }
        void
        cdata(const void* cdata, unsigned csize)
        {
        }
        void
        end(bool commit)
        {
        }
    };

    struct marhandler : basic_marnonstatic {
        void
        message(const char* initial, aug_mar_t mar)
        {
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

        httphandler x;
        httpparser hparser(mp, 1024, x);

        marhandler y;
        marparser mparser(mp, 1024, y);

        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
