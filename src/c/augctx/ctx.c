/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/ctx.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/errinfo.h"
#include "augctx/utility.h" /* aug_timezone() */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
# include <winsock2.h>
#endif /* _WIN32 */

struct impl_ {
    aug_ctx ctx_;
    int refs_;
    aug_mpool* mpool_;
    aug_clock* clock_;
    aug_log* log_;
    int loglevel_;
    struct aug_errinfo errinfo_;
};

static void*
cast_(aug_ctx* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_ctxid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_ctx* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_ctx* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_clock* clock = impl->clock_;
        aug_log* log = impl->log_;
        aug_free(mpool, impl);
        aug_release(log);
        aug_release(clock);
        aug_release(mpool);
#if defined(_WIN32)
        WSACleanup();
#endif /* _WIN32 */
    }
}

static void
setlog_(aug_ctx* obj, aug_log* log)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    assert(log);
    /* Retain before release - avoiding issues with self assignment. */
    aug_retain(log);
    aug_release(impl->log_);
    impl->log_ = log;
}

static int
setloglevel_(aug_ctx* obj, int level)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    int prev = impl->loglevel_;
    impl->loglevel_ = level;
    return prev;
}

static aug_mpool*
getmpool_(aug_ctx* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    aug_retain(impl->mpool_);
    return impl->mpool_;
}

static aug_clock*
getclock_(aug_ctx* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    aug_retain(impl->clock_);
    return impl->clock_;
}

static aug_log*
getlog_(aug_ctx* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    aug_retain(impl->log_);
    return impl->log_;
}

static int
getloglevel_(aug_ctx* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    return impl->loglevel_;
}

static struct aug_errinfo*
geterrinfo_(aug_ctx* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    return &impl->errinfo_;
}

static const struct aug_ctxvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    setlog_,
    setloglevel_,
    getmpool_,
    getclock_,
    getlog_,
    getloglevel_,
    geterrinfo_
};

static aug_result
vctxlog_(aug_ctx* ctx, int level, const char* format, va_list args)
{
    aug_result result = AUG_SUCCESS;
    assert(ctx);
    if (level <= aug_getloglevel(ctx)) {
        aug_log* log = aug_getlog(ctx);
        if (log) {
            result = aug_vwritelog(log, level, format, args);
            aug_release(log);
        }
    }
    return result;
}

AUGCTX_API aug_ctx*
aug_createctx(aug_mpool* mpool, aug_clock* clock, aug_log* log, int loglevel)
{
    struct impl_* impl;
    assert(mpool);
    assert(clock);
    assert(log);

#if defined(_WIN32)
    {
        WSADATA data;
        int err = WSAStartup(MAKEWORD(2, 2), &data);
        if (0 != err)
            return NULL;
    }
#endif /* _WIN32 */

    if (!(impl = aug_malloc(mpool, sizeof(struct impl_)))) {
#if defined(_WIN32)
        WSACleanup();
#endif /* _WIN32 */
        return NULL;
    }

    impl->ctx_.vtbl_ = &vtbl_;
    impl->ctx_.impl_ = NULL;
    impl->refs_ = 1;

    aug_retain(mpool);
    aug_retain(clock);
    aug_retain(log);

    impl->mpool_ = mpool;
    impl->clock_ = clock;
    impl->log_ = log;

    impl->loglevel_ = loglevel;
    memset(&impl->errinfo_, 0, sizeof(impl->errinfo_));

    return &impl->ctx_;
}

AUGCTX_API aug_ctx*
aug_createbasicctx(void)
{
    long tz;
    aug_mpool* mpool;
    aug_clock* clock;
    aug_log* log;
    aug_ctx* ctx = NULL;

    if (!aug_timezone(&tz))
        return NULL;

    if (!(mpool = aug_createdlmalloc()))
        return NULL;

    if (!(clock = aug_createclock(mpool, tz)))
        goto fail1;

    if (!(log = aug_createstdlog(mpool)))
        goto fail2;

    ctx = aug_createctx(mpool, clock, log, aug_loglevel());

    aug_release(log);
 fail2:
    aug_release(clock);
 fail1:
    aug_release(mpool);
    return ctx;
}

AUGCTX_API aug_result
aug_vctxlog(aug_ctx* ctx, int level, const char* format, va_list args)
{
    return vctxlog_(ctx, level, format, args);
}

AUGCTX_API aug_result
aug_ctxlog(aug_ctx* ctx, int level, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, level, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxcrit(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGCRIT, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxerror(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGERROR, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxwarn(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGWARN, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxnotice(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGNOTICE, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxinfo(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGINFO, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxdebug0(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGDEBUG0, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxdebug1(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGDEBUG0 + 1, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxdebug2(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGDEBUG0 + 2, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_ctxdebug3(aug_ctx* ctx, const char* format, ...)
{
    aug_result result;
    va_list args;
    va_start(args, format);
    result = vctxlog_(ctx, AUG_LOGDEBUG0 + 3, format, args);
    va_end(args);
    return result;
}

AUGCTX_API aug_result
aug_perrinfo(aug_ctx* ctx, const char* s)
{
    struct aug_errinfo* errinfo = aug_geterrinfo(ctx);
    const char* file;

    if (0 == errinfo->num_) {
        aug_ctxerror(ctx, "%s: no description available", s);
        return 0;
    }

    for (file = errinfo->file_; ; ++file)
        switch (*file) {
        case '.':
        case '/':
        case '\\':
            break;
        default:
            goto done;
        }
 done:
    return aug_ctxerror(ctx, "%s:%d: %s: [src=%s, num=%d (0x%.8x)] %s",
                        file, (int)errinfo->line_, s, errinfo->src_,
                        (int)errinfo->num_, (int)errinfo->num_,
                        errinfo->desc_);
}
