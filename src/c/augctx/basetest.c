/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

#include <errno.h>
#include <stdio.h>

int
main(int argc, char* argv[])
{
    /* Ensure that the initialisation functions can be called multiple
       times. */

    if (aug_init() < 0 || aug_init() < 0 || aug_init() < 0) {
        fprintf(stderr, "aug_init() failed\n");
        return 1;
    }

    aug_term();
    aug_term();
    aug_term();

    if (aug_autobasictlx() < 0)
        return 1;

    aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);
    aug_perrinfo(aug_tlx, "operation failed", NULL);
    return 0;
}
