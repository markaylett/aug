/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_BASE_H
#define AUGCTX_BASE_H

#include "augctx/ctx.h"
#include "augbool.h"

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
 * aug_init(), as some library functions may depend on it.
 *
 * @return @ref AUG_TRUE on success, otherwise @ref AUG_FALSE.
 */

AUGCTX_API aug_bool
aug_init(void);

/**
 * Terminate use of library.
 *
 * Informs the library that the calling thread has finished with it, and that
 * associated resources can may now be freed.
 *
 * When freeing resources, aug_term() will also call aug_release() on the
 * thread-local context.
 */

AUGCTX_API void
aug_term(void);

/**
 * @defgroup Logging Logging
 *
 * Core logging functions.
 *
 * The log request will only be actioned when @a level is less than or equal
 * to the log-level associated with the context.
 *
 * @{
 */

AUGCTX_API int
aug_vctxlog(aug_ctx* ctx, int level, const char* format, va_list args);

AUGCTX_API int
aug_ctxlog(aug_ctx* ctx, int level, const char* format, ...);

/** @} */

/**
 * @defgroup LoggingWrappers Wrappers
 * @ingroup Logging
 *
 * The following functions are convenience wrappers around aug_vctxlog().
 *
 * @{
 */

AUGCTX_API int
aug_ctxcrit(aug_ctx* ctx, const char* format, ...);

AUGCTX_API int
aug_ctxerror(aug_ctx* ctx, const char* format, ...);

AUGCTX_API int
aug_ctxwarn(aug_ctx* ctx, const char* format, ...);

AUGCTX_API int
aug_ctxnotice(aug_ctx* ctx, const char* format, ...);

AUGCTX_API int
aug_ctxinfo(aug_ctx* ctx, const char* format, ...);

/** @} */

/**
 * Greater debug levels are supported by calling aug_ctxlog() directly.
 */

AUGCTX_API int
aug_ctxdebug0(aug_ctx* ctx, const char* format, ...);

AUGCTX_API int
aug_ctxdebug1(aug_ctx* ctx, const char* format, ...);

AUGCTX_API int
aug_ctxdebug2(aug_ctx* ctx, const char* format, ...);

AUGCTX_API int
aug_ctxdebug3(aug_ctx* ctx, const char* format, ...);

#if !defined(NDEBUG)
# define AUG_CTXDEBUG0 aug_ctxdebug0
# define AUG_CTXDEBUG1 aug_ctxdebug1
# define AUG_CTXDEBUG2 aug_ctxdebug2
# define AUG_CTXDEBUG3 aug_ctxdebug3
#else /* NDEBUG */
# define AUG_CTXDEBUG0 1 ? (void)0 : (void)aug_ctxdebug0
# define AUG_CTXDEBUG1 1 ? (void)0 : (void)aug_ctxdebug1
# define AUG_CTXDEBUG2 1 ? (void)0 : (void)aug_ctxdebug2
# define AUG_CTXDEBUG3 1 ? (void)0 : (void)aug_ctxdebug3
#endif /* NDEBUG */

AUGCTX_API int
aug_perrinfo(aug_ctx* ctx, const char* s);

/**
 * Set thread-local context.
 *
 * The context will be retained in thread-local storage.
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
