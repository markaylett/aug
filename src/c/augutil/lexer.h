/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_LEXER_H
#define AUGUTIL_LEXER_H

#include "augutil/config.h"
#include "augsys/types.h"

enum aug_token {

    AUG_TOKERROR = -1,
    AUG_TOKAGAIN = 0,

    /* Delimiter. */

    AUG_TOKDELIM,

    /* Logical word. */

    AUG_TOKWORD,

    /* Boundary. */

    AUG_TOKBOUND
};

typedef int (*aug_isdelim_t)(char);
typedef struct aug_lexer_* aug_lexer_t;

AUGUTIL_API aug_lexer_t
aug_createlexer(size_t size, aug_isdelim_t isdelim);

AUGUTIL_API int
aug_destroylexer(aug_lexer_t lexer);

AUGUTIL_API enum aug_token
aug_appendlexer(aug_lexer_t* lexer, char ch);

AUGUTIL_API int
aug_finishlexer(aug_lexer_t* lexer);

AUGUTIL_API aug_isdelim_t
aug_setlexerdelim(aug_lexer_t lexer, aug_isdelim_t isdelim);

AUGUTIL_API const char*
aug_lexertoken(aug_lexer_t lexer);

typedef struct aug_lexer2_* aug_lexer2_t;

AUGUTIL_API aug_lexer2_t
aug_createlexer2(size_t size, void (*out)(void*, int, const char*),
                 void* arg);

AUGUTIL_API int
aug_destroylexer2(aug_lexer2_t lexer);

AUGUTIL_API void
aug_appendlexer2(aug_lexer2_t lexer, char ch);

AUGUTIL_API void
aug_finishlexer2(aug_lexer2_t lexer);

#endif /* AUGUTIL_LEXER_H */
