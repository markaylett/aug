/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_BASE_H
#define AUGCTX_BASE_H

#include "augctx/ctx.h"
#include "augtypes.h"

/**
 * @file augctx/base.h
 *
 * Context functions.
 */

/**
 * Initialise library for calling thread.
 *
 * Multiple calls to aug_init() can be safely made from a single thread.  This
 * allows initialisation from functions such as DllMain().  Each call must be
 * matched by a call to aug_term() when the thread is finished with the
 * library.  An internal reference count tracks multiple calls, and
 * termination only happens when this count reaches zero.
 *
 * A thread must ensure the thread-local context is set after calling
 * aug_init() as some library functions may depend on it.
 *
 * @return Either @ref AUG_SUCCESS or @ref AUG_FAILURE.
 */

AUGCTX_API aug_result
aug_init(void);

AUGCTX_API aug_result
aug_initbasicctx(void);

/**
 * Terminate use of library.
 *
 * Informs the library that the calling thread has finished with it, and that
 * any associated resources may now be freed.
 *
 * When freeing resources, aug_term() will release the thread-local context.
 */

AUGCTX_API void
aug_term(void);

/**
 * Set thread-local context.
 *
 * The context will be retained in thread-local storage.
 *
 * @param ctx The context to be retained.
 */

AUGCTX_API void
aug_settlx(aug_ctx* ctx);

/**
 * Obtain reference to thread-local context.
 *
 * The caller must release the reference when finished with it.
 *
 * @return A retained context reference.
 */

AUGCTX_API aug_ctx*
aug_gettlx(void);

/**
 * Obtain reference to thread-local context, without retaining a reference to
 * it.
 *
 * The returned reference is not retained.  The caller must _not_, therefore,
 * release it.  This function can be used to test the existence of the
 * thread-local context.
 *
 * @return A borrowed context reference.
 */

#define aug_tlx aug_tlx_()

AUGCTX_API aug_ctx*
aug_tlx_(void);

#endif /* AUGCTX_BASE_H */
