/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augutil.h"

#include <stdio.h>
#include <stdlib.h>

static char buf_[256];
static unsigned i_ = 0;

static void
out_(void* arg, int what)
{
    switch (what) {
    case TOKLABEL:
        buf_[i_++] = '\0';
        printf("'%s'=", buf_);
        i_ = 0;
        break;
    case TOKWORD:
        buf_[i_++] = '\0';
        printf("[%s]", buf_);
        i_ = 0;
        break;
    case TOKPHRASE:
        printf("\n");
        break;
    default:
        buf_[i_++] = what;
        break;
    }
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    struct aug_words st;
    int ch;

    aug_atexitinit(&errinfo);
    aug_initshellwords(&st, out, NULL);

    while (EOF != (ch = getchar()))
        aug_putshellwords(&st, ch);
    aug_putshellwords(&st, '\n');
    return 0;
}
