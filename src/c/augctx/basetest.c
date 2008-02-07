/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

static aug_ctx*
createctx_(long tz, int level)
{
    aug_mpool* mpool;
    aug_clock* clock;
    aug_log* log;
    aug_ctx* ctx;

    mpool = aug_createdlmalloc();
    clock = aug_createclock(mpool, tz);
    log = aug_createstdlog(mpool);
    ctx = aug_createctx(mpool, clock, log, level);

    aug_release(log);
    aug_release(clock);
    aug_release(mpool);

    return ctx;
}

int
main(int argc, char* argv[])
{
    aug_init();
    aug_init();
    if (!aug_havectx()) {
        long tz;
        aug_ctx* ctx;
        aug_timezone(&tz);
        ctx = createctx_(tz, aug_loglevel());
        aug_setctx(ctx);
        aug_release(ctx);
    }
    aug_writectx(NULL, AUG_LOGINFO, "%s", "a test message");
    aug_term();
    aug_term();

    return 0;
}
