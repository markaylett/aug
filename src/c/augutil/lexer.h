/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
