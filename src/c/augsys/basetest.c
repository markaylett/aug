/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    struct aug_errinfo a, b, c;

    /* Ensure that the initialisation functions can be called multiple
       times. */

    if (-1 == aug_term()) {
        aug_perror("aug_term() failed");
        return 1;
    }

    if (-1 == aug_init(&a) || -1 == aug_init(&b) || -1 == aug_init(&c)) {
        aug_perror("aug_init() failed");
        return 1;
    }

    if (aug_geterrinfo() != &a) {
        fprintf(stderr, "unexpected return value from aug_geterrinfo()\n");
        return 1;
    }

    if (-1 == aug_term() || -1 == aug_term() || -1 == aug_term()) {
        aug_perror("aug_term() failed");
        return 1;
    }

    /* One too many: should be ignored. */

    if (-1 == aug_term()) {
        aug_perror("aug_term() failed");
        return 1;
    }

    return 0;
}
