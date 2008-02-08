/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/lock.h"
#include "augctx/tls_.h"

#include <assert.h>
#include <stdlib.h> /* malloc() */

struct tls_ {
    int refs_;
    aug_ctx* ctx_;
};

#if ENABLE_THREADS

# include "augctx/tls_.h"

static aug_bool init_ = AUG_FALSE;
static aug_tlskey_t tlskey_;

static struct tls_*
gettls_(void)
{
    return aug_gettlsvalue_(tlskey_);
}

static aug_bool
inittls_(void)
{
    /* Lock held. */

    struct tls_* tls;
    if (!init_) {

        /* TLS key must be initialised on first call. */

        if (!aug_createtlskey_(&tlskey_))
            return AUG_FALSE;

        init_ = AUG_TRUE;
        goto skip;
    }

    if (!(tls = gettls_())) {
    skip:
        /* First call on this thread. */

        if (!(tls = malloc(sizeof(struct tls_))))
            return AUG_FALSE;

        tls->refs_ = 0;
        tls->ctx_ = NULL;
        aug_settlsvalue_(tlskey_, tls);
    }
    ++tls->refs_;
    return AUG_TRUE;
}

static void
termtls_(void)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (0 == --tls->refs_) {

        /* Free resources associated with thread. */

        aug_settlsvalue_(tlskey_, NULL);
        if (tls->ctx_) {
            aug_release(tls->ctx_);
            tls->ctx_ = NULL;
        }
        free(tls);
    }
}

#else /* !ENABLE_THREADS */

/* Single-threaded. */

static struct tls_ tls_ = { 0, NULL };

static struct tls_*
gettls_(void)
{
    return &tls_;
}

static aug_bool
inittls_(void)
{
    ++tls_.refs_;
    return AUG_TRUE;
}

static void
termtls_(void)
{
    assert(0 < tls_.refs_);
    if (0 == --tls_.refs_ && tls_.ctx_) {
        aug_release(tls_.ctx_);
        tls_.ctx_ = NULL;
    }
}

#endif /* !ENABLE_THREADS */

static int
vctxlog_(aug_ctx* ctx, int level, const char* format, va_list args)
{
    int ret = 0;
    assert(ctx);
    if (level <= aug_getloglevel(ctx)) {
        aug_log* log = aug_getlog(ctx);
        if (log) {
            ret = aug_vwritelog(log, level, format, args);
            aug_release(log);
        }
    }
    return ret;
}

AUGCTX_API aug_bool
aug_init(void)
{
    aug_bool ret = aug_initlock_();
    if (ret) {
        aug_lock();
        ret = inittls_();
        aug_unlock();
    }
    return ret;
}

AUGCTX_API void
aug_term(void)
{
    termtls_();
}

AUGCTX_API void
aug_setctx(aug_ctx* ctx)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    /* Retain before release - avoiding issues with self assignment. */
    if (ctx)
        aug_retain(ctx);
    if (tls->ctx_)
        aug_release(tls->ctx_);
    tls->ctx_ = ctx;
}

AUGCTX_API aug_ctx*
aug_getctx(void)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (tls->ctx_)
        aug_retain(tls->ctx_);
    return tls->ctx_;
}

AUGCTX_API aug_ctx*
aug_usectx(void)
{
    return gettls_()->ctx_;
}

AUGCTX_API int
aug_vctxlog(aug_ctx* ctx, int level, const char* format, va_list args)
{
    if (!ctx)
        ctx = aug_usectx();
    return ctx ? vctxlog_(ctx, level, format, args) : 0;
}

AUGCTX_API int
aug_ctxlog(aug_ctx* ctx, int level, const char* format, ...)
{
    int ret;
    va_list args;
    if (!ctx)
        ctx = aug_usectx();
    if (ctx) {
        va_start(args, format);
        ret = vctxlog_(ctx, level, format, args);
        va_end(args);
    } else
        ret = 0;
    return ret;
}

AUGCTX_API int
aug_ctxcrit(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGCRIT, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxerror(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGERROR, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxwarn(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGWARN, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxnotice(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGNOTICE, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxinfo(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGINFO, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxdebug0(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGDEBUG0, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxdebug1(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGDEBUG0 + 1, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxdebug2(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGDEBUG0 + 2, format, args);
    va_end(args);
    return ret;
}

AUGCTX_API int
aug_ctxdebug3(aug_ctx* ctx, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vctxlog(ctx, AUG_LOGDEBUG0 + 3, format, args);
    va_end(args);
    return ret;
}
