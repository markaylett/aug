/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    aug_setposixerrinfo(NULL, __FILE__, 101, EINVAL);

    if (0 != strcmp(aug_errfile, __FILE__)) {
        fprintf(stderr, "unexpected aug_errfile value: %s\n", aug_errfile);
        return 1;
    }

    if (101 != aug_errline) {
        fprintf(stderr, "unexpected aug_errline value: %d\n",
                (int)aug_errline);
        return 1;
    }

    if (AUG_SRCPOSIX != aug_errsrc) {
        fprintf(stderr, "unexpected aug_errsrc value: %d\n", (int)aug_errsrc);
        return 1;
    }

    if (0 != strcmp(aug_errdesc, strerror(EINVAL))) {
        fprintf(stderr, "unexpected aug_errdesc value: %s\n", aug_errdesc);
        return 1;
    }

    return 0;
}
