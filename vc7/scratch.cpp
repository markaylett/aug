/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsyspp.hpp"

int
main(int argc, char* argv[])
{
    try {

        autodltlx();
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
