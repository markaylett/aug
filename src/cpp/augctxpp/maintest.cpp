/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {
    class mpool_base {
    protected:
        ~mpool_base()
        {
        }
    public:
        static void*
        operator new(size_t size)
        {
            mpoolptr mpool(getmpool(aug_tlx));
            return aug_allocmem(mpool.get(), size);
        }
        static void
        operator delete(void* ptr)
        {
            mpoolptr mpool(getmpool(aug_tlx));
            aug_freemem(mpool.get(), ptr);
        }
    };
    class test : public mpool_base {
    };
}

int
main(int argc, char* argv[])
{
    try {

        autobasictlx();
        test* t = new test();
        delete t;
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
