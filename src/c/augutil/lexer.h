/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_LEXER_H
#define AUGUTIL_LEXER_H

/**
 * @file augutil/lexer.h
 *
 * Lexical tokeniser.
 */

#include "augutil/config.h"

#include "augsys/types.h"

#include "augext/mpool.h"

#define AUG_LEXLABEL  0x01
#define AUG_LEXWORD   0x02
#define AUG_LEXPHRASE 0x04

#define AUG_LEXWORDPHRASE (AUG_LEXWORD | AUG_LEXPHRASE)

typedef struct aug_lexer_* aug_lexer_t;

AUGUTIL_API aug_lexer_t
aug_createnetlexer(aug_mpool* mpool, size_t size);

AUGUTIL_API aug_lexer_t
aug_createshelllexer(aug_mpool* mpool, size_t size, int pairs);

AUGUTIL_API void
aug_destroylexer(aug_lexer_t lexer);

AUGUTIL_API unsigned
aug_appendlexer(aug_lexer_t lexer, char ch);

AUGUTIL_API unsigned
aug_finishlexer(aug_lexer_t lexer);

AUGUTIL_API const char*
aug_lexertoken(aug_lexer_t lexer);

#endif /* AUGUTIL_LEXER_H */
