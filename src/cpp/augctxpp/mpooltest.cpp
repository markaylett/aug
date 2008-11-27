/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augctxpp.hpp"

#include <iostream>
#include <memory>

using namespace aug;
using namespace std;

namespace {
    class foo : public mpool_ops {
        char pad_[11]; // Prime.
    };
    class bar : public mpool_ops {
        char pad_[23]; // Prime.
    };
    class foobar : public foo, public bar {
        char pad_[31]; // Prime.
    };

    // Non-trivial allocation sequence.

    void
    createfoo()
    {
        delete[] new (tlx) foo[64];
    }

    void
    createbar()
    {
        auto_ptr<bar> p1(new (tlx) bar());
        createfoo();
        auto_ptr<bar> p2(new (tlx) bar());
        createfoo();
    }

    void
    test()
    {
        auto_ptr<foobar> p1(new (tlx) foobar());
        createbar();
        auto_ptr<foobar> p2(new (tlx) foobar());
        createbar();
    }
}

int
main(int argc, char* argv[])
{
    try {

        scoped_init scoped(null);

        try {

            setbasictlx(createdlmalloc());
            //setbasictlx(getcrtmalloc());

            for (int i(0); i < 1000; ++i)
                test();

            delete[] new (tlx) foobar[64];
            return 0;

        } AUG_PERRINFOCATCH;
    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
    }
    return 1;
}
