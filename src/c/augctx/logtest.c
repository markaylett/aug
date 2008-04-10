/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#if !defined(NDEBUG)
# define NDEBUG
#endif /* !NDEBUG */

#include "augctx.h"

#include <stdio.h>

static int logged_;

static int
logger_(int loglevel, const char* format, va_list args)
{
    logged_ = 1;
    return 0;
}

int
main(int argc, char* argv[])
{
    if (aug_start() < 0)
        return 1;

    aug_setlogger(logger_);
    aug_setloglevel(AUG_LOGINFO);

    logged_ = 0;
    aug_info("this should be logged");
    if (!logged_) {
        fprintf(stderr, "message not logged\n");
        return 1;
    }

    logged_ = 0;
    aug_debug0("this should not be logged");
    if (logged_) {
        fprintf(stderr, "message logged\n");
        return 1;
    }

    {
        int i = 101;
        AUG_DEBUG0("not evaluated: %d", (i = 202));
        if (202 == i) {
            fprintf(stderr, "evaluation of debug trace\n");
            return 1;
        }
    }
    return 0;
}
