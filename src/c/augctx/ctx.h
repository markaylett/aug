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
#ifndef AUGCTX_CTX_H
#define AUGCTX_CTX_H

#include "augctx/config.h"

#include "augext/ctx.h"

/**
 * Neither @a clock nor @a log should reference the thread-local context.
 */

AUGCTX_API aug_ctx*
aug_createctx(aug_mpool* mpool, aug_clock* clock, aug_log* log,
              unsigned level);

AUGCTX_API aug_ctx*
aug_createbasicctx(aug_mpool* mpool);

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

AUGCTX_API aug_bool
aug_vctxlog(aug_ctx* ctx, unsigned level, const char* format, va_list args);

AUGCTX_API aug_bool
aug_ctxlog(aug_ctx* ctx, unsigned level, const char* format, ...);

/** @} */

/**
 * @defgroup LoggingWrappers Wrappers
 * @ingroup Logging
 *
 * The following functions are convenience wrappers around aug_vctxlog().
 *
 * @{
 */

AUGCTX_API aug_bool
aug_ctxcrit(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_bool
aug_ctxerror(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_bool
aug_ctxwarn(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_bool
aug_ctxnotice(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_bool
aug_ctxinfo(aug_ctx* ctx, const char* format, ...);

/** @} */

/**
 * Greater debug levels are supported by calling aug_ctxlog() directly.
 */

AUGCTX_API aug_bool
aug_ctxdebug0(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_bool
aug_ctxdebug1(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_bool
aug_ctxdebug2(aug_ctx* ctx, const char* format, ...);

AUGCTX_API aug_bool
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

AUGCTX_API aug_bool
aug_perrinfo(aug_ctx* ctx, const char* s, const struct aug_errinfo* errinfo);

#endif /* AUGCTX_CTX_H */
