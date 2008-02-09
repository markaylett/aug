/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char* argv[])
{
    aug_init();
    aug_init();
    if (!aug_tlx) {
        long tz;
        aug_ctx* ctx;
        aug_timezone(&tz);
        ctx = aug_createbasicctx(tz, aug_loglevel());
        aug_settlx(ctx);
        aug_release(ctx);
    }

    aug_setposixerrinfo(aug_geterrinfo(aug_tlx), __FILE__, __LINE__, ENOMEM);
    aug_perrinfo(aug_tlx, "operation failed");
    aug_term();
    aug_term();

    return 0;
}
