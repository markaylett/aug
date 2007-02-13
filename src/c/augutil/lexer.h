/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_LEXER_H
#define AUGUTIL_LEXER_H

#include "augutil/config.h"
#include "augsys/types.h"

enum aug_token {

    AUG_TOKERROR = -1,
    AUG_TOKNONE = 0,

    /* Delimited. */

    AUG_TOKDELIM,

    /* Logical line. */

    AUG_TOKLINE,
    AUG_TOKBREAK
};

typedef int (*aug_isdelim_t)(char);
typedef struct aug_lexer_* aug_lexer_t;

AUGUTIL_API aug_lexer_t
aug_createlexer(size_t size, aug_isdelim_t isdelim);

AUGUTIL_API int
aug_destroylexer(aug_lexer_t lexer);

AUGUTIL_API enum aug_token
aug_lexchar(aug_lexer_t* lexer, char ch);

AUGUTIL_API int
aug_lexend(aug_lexer_t* lexer);

AUGUTIL_API aug_isdelim_t
aug_setisdelim(aug_lexer_t lexer, aug_isdelim_t isdelim);

AUGUTIL_API const char*
aug_token(aug_lexer_t lexer);

#endif /* AUGUTIL_LEXER_H */
