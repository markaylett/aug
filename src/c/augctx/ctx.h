/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_CTX_H
#define AUGCTX_CTX_H

#include "augctx/config.h"

#include "augext/ctx.h"

AUGCTX_API aug_ctx*
aug_createctx(aug_mpool* mpool, aug_clock* clock, aug_log* log, int level);

AUGCTX_API aug_ctx*
aug_createbasicctx(void);

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

AUGCTX_API aug_result
aug_vctxlog(aug_ctx* ctx, int level, const char* format, va_list args);

AUGCTX_API aug_result
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

AUGCTX_API aug_result
aug_ctxcrit(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_result
aug_ctxerror(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_result
aug_ctxwarn(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_result
aug_ctxnotice(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_result
aug_ctxinfo(aug_ctx* ctx, const char* format, ...);

/** @} */

/**
 * Greater debug levels are supported by calling aug_ctxlog() directly.
 */

AUGCTX_API aug_result
aug_ctxdebug0(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_result
aug_ctxdebug1(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_result
aug_ctxdebug2(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_result
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

AUGCTX_API aug_result
aug_perrinfo(aug_ctx* ctx, const char* s, const struct aug_errinfo* errinfo);

#endif /* AUGCTX_CTX_H */
