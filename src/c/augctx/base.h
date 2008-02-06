/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_BASE_H
#define AUGCTX_BASE_H

#include "augctx/ctx.h"
#include "augctx/types.h"

AUGCTX_API aug_bool
aug_init(void);

AUGCTX_API void
aug_term(void);

AUGCTX_API void
aug_setctx(aug_ctx* ctx);

/**
 * Obtain reference to thread-local context.
 *
 * The caller must call aug_release() when finished with the context.
 *
 * @return A retained context reference.
 */

AUGCTX_API aug_ctx*
aug_getctx(void);

/**
 * Obtain reference to thread-local context, without retaining a reference to
 * it.
 *
 * The caller must _not_ call aug_release() on the context.
 *
 * @return A borrowed context reference.
 */

AUGCTX_API aug_ctx*
aug_usectx(void);

#define aug_havectx() (aug_usectx() ? AUG_TRUE : AUG_FALSE)

#endif /* AUGCTX_BASE_H */
