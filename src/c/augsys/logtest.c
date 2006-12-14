/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

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
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

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

#if !defined(NDEBUG)
# define NDEBUG
#endif /* !NDEBUG */
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
