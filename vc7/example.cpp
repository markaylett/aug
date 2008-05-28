/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsyspp.hpp"

int
main(int argc, char* argv[])
{
    try {

        atbasixtlx();
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
