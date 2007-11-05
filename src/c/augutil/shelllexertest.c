/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augutil.h"

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
    struct aug_errinfo errinfo;
    char ch;
    aug_atexitinit(&errinfo);

    lexer_ = aug_createshelllexer(0);
    assert(lexer_);
    while (EOF != (ch = getchar()))
        out_(aug_appendlexer(lexer_, ch));
    out_(aug_finishlexer(lexer_));
    aug_destroylexer(lexer_);

    return 0;
}
