/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_BASE_H
#define AUGCTX_BASE_H

#include "augctx/ctx.h"

AUGCTX_API void
aug_init(void);

AUGCTX_API void
aug_term(void);

AUGCTX_API void
aug_setctx(aug_ctx* ctx);

AUGCTX_API aug_ctx*
aug_getctx(void);

#endif /* AUGCTX_BASE_H */
