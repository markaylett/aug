/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {
    class foo : public mpool_base { };
    class bar : public mpool_base { };
    class foobar : public foo, public bar { };
}

int
main(int argc, char* argv[])
{
    try {

        autobasictlx();
        for (int i(0); i < 1000; ++i)
            delete new foobar();
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
