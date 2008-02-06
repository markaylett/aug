/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

int
main(int argc, char* argv[])
{
    aug_init();
    aug_init();
    aug_term();
    aug_term();
    return 0;
}
