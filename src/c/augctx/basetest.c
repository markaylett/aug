/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char* argv[])
{
    aug_init();
    aug_init();
    if (!aug_usectx()) {
        long tz;
        aug_ctx* ctx;
        aug_timezone(&tz);
        ctx = aug_createbasicctx(tz, aug_loglevel());
        aug_setctx(ctx);
        aug_release(ctx);
    }
    aug_ctxinfo(NULL, "%s", "a test message");
    aug_term();
    aug_term();

    return 0;
}
