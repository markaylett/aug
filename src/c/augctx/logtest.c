/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/

#if !defined(NDEBUG)
# define NDEBUG
#endif /* !NDEBUG */

#include "augctx.h"

#include <stdio.h>

static int logged_;

static void*
cast_(aug_log* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_logid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_log* obj)
{
}

static void
release_(aug_log* obj)
{
}

static aug_result
vwritelog_(aug_log* obj, int level, const char* format, va_list args)
{
    logged_ = 1;
    return AUG_SUCCESS;
}

static const struct aug_logvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    vwritelog_
};

static aug_log log_ = { &vtbl_, NULL };

int
main(int argc, char* argv[])
{
    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    aug_setlog(aug_tlx, &log_);
    aug_setloglevel(aug_tlx, AUG_LOGINFO);

    logged_ = 0;
    aug_ctxinfo(aug_tlx, "this should be logged");
    if (!logged_) {
        fprintf(stderr, "message not logged\n");
        return 1;
    }

    logged_ = 0;
    aug_ctxdebug0(aug_tlx, "this should not be logged");
    if (logged_) {
        fprintf(stderr, "message logged\n");
        return 1;
    }

    {
        int i = 101;
        AUG_CTXDEBUG0(aug_tlx, "not evaluated: %d", (i = 202));
        if (202 == i) {
            fprintf(stderr, "evaluation of debug trace\n");
            return 1;
        }
    }
    return 0;
}
