/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

#include <errno.h> /* ENOMEM */
#include <stdio.h>

int
main(int argc, char* argv[])
{
    /* Ensure that the initialisation functions can be called multiple
       times. */

    if (AUG_ISFAIL(aug_init()) || AUG_ISFAIL(aug_init())
        || AUG_ISFAIL(aug_init())) {
        fprintf(stderr, "aug_init() failed\n");
        return 1;
    }

    aug_term();
    aug_term();
    aug_term();

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);
    aug_perrinfo(aug_tlx, "operation failed", NULL);
    return 0;
}
