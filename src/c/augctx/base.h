/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_BASE_H
#define AUGCTX_BASE_H

#include "augctx/ctx.h"
#include "augbool.h"

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

AUGCTX_API int
aug_vctxlog(aug_ctx* ctx, int level, const char* format, va_list args);

AUGCTX_API int
aug_ctxlog(aug_ctx* ctx, int level, const char* format, ...);

/**
 * @defgroup LoggingWrappers Wrappers
 * @ingroup Logging
 *
 * The following functions are essentially convenience wrappers around
 * aug_vwritelog().
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
 * Guidelines for debug-level use:
 *
 * aug_ctxdebug0() - user applications;
 * aug_ctxdebug1() - user applications;
 * aug_ctxdebug2() - aug applications (such as daug and mar);
 * aug_ctxdebug3() - aug libraries.
 *
 * Note: further levels can be used by calling aug_ctxlog() directly.
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

#endif /* AUGCTX_BASE_H */
