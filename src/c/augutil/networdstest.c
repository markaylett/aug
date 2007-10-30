/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augutil.h"

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
    case AUG_TOKPHRASE:
        printf("\n");
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
    struct aug_errinfo errinfo;
    struct aug_words st;
    int ch;

    aug_atexitinit(&errinfo);
    aug_initshellwords(&st, out_, NULL);

    while (EOF != (ch = getchar()))
        aug_putshellwords(&st, ch);
    aug_putshellwords(&st, '\n');
    return 0;
}

#if 0
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
        printf("[%s]\n", buf_);
        i_ = 0;
        break;
    case TOKPHRASE:
        printf("--\n");
        break;
    case TOKRTRIM:
        i_ = rtrim_(buf_, i_);
        break;
    default:
        buf_[i_++] = what;
        break;
    }
}

int
main(int argc, char* argv[])
{
    struct state st;
    init_(&st, out_, NULL);
    int ch;
    while (EOF != (ch = getchar()))
        append_(&st, ch);
    append_(&st, '\n');
    return 0;
}
#endif
