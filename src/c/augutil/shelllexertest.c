/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"

#include <assert.h>
#include <stdio.h>

static aug_lexer_t lexer_ = NULL;

static void
out_(unsigned flags)
{
    switch (flags) {
    case AUG_LEXLABEL:
        printf("'%s'=", aug_lexertoken(lexer_));
        break;
    case AUG_LEXWORD:
        printf("[%s]", aug_lexertoken(lexer_));
        break;
    case AUG_LEXWORD | AUG_LEXPHRASE:
        printf("[%s]", aug_lexertoken(lexer_));
    case AUG_LEXPHRASE:
        printf("\n");
        break;
    }
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    char ch;
    if (aug_autobasictlx() < 0)
        return 1;

    mpool = aug_getmpool(aug_tlx);
    lexer_ = aug_createshelllexer(mpool, 0, 1);
    aug_release(mpool);

    assert(lexer_);
    while (EOF != (ch = getchar()))
        out_(aug_appendlexer(lexer_, ch));
    out_(aug_finishlexer(lexer_));
    aug_destroylexer(lexer_);

    return 0;
}
