/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static char buf_[256];
static unsigned i_ = 0;

static void
out_(void* arg, int what)
{
    switch (what) {
    case AUG_TOKERROR:
        assert(0);
    case AUG_TOKPHRASE:
        printf("\n");
        break;
    case AUG_TOKLABEL:
        buf_[i_++] = '\0';
        printf("'%s'=", buf_);
        i_ = 0;
        break;
    case AUG_TOKWORD:
        buf_[i_++] = '\0';
        printf("[%s]", buf_);
        i_ = 0;
        break;
    case AUG_TOKRTRIM:
        assert(0);
    default:
        buf_[i_++] = what;
        break;
    }
}

int
main(int argc, char* argv[])
{
    struct aug_words st;
    int ch;

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;
    aug_initshellwords(&st, 1, out_, NULL);

    while (EOF != (ch = getchar()))
        aug_putshellwords(&st, ch);
    aug_putshellwords(&st, '\n');
    return 0;
}
