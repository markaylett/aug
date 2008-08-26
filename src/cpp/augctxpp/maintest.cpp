/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctxpp.hpp"

using namespace aug;

int
main(int argc, char* argv[])
{
    try {

        autobasictlx();
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
