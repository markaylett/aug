/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

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
    if (!aug_autotlx())
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
