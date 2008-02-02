/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/ctx.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/types.h"

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
    int level_;
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
    aug_release(impl->log_);
    aug_retain(log);
    impl->log_ = log;
}

static int
setloglevel_(aug_ctx* obj, int level)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    int prev = impl->level_;
    impl->level_ = level;
    return prev;
}

static int
vwritelog_(aug_ctx* obj, int level, const char* format, va_list args)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, ctx_, obj);
    if (level <= impl->level_)
        return aug_vprintlog(impl->log_, level, format, args);
    return 0;
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
    return impl->level_;
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
    vwritelog_,
    getmpool_,
    getclock_,
    getlog_,
    getloglevel_,
    geterrinfo_
};

AUGCTX_API aug_ctx*
aug_createctx(aug_mpool* mpool, aug_clock* clock, aug_log* log, int level)
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

    impl->level_ = level;
    memset(&impl->errinfo_, 0, sizeof(impl->errinfo_));

    return &impl->ctx_;
}

AUGCTX_API int
aug_loglevel(void)
{
    const char* s = getenv("AUG_LOGLEVEL");
    return s ? atoi(s) : AUG_LOGINFO;
}

AUGCTX_API int
aug_writelog(aug_ctx* ctx, int level, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(ctx, level, format, args);
    va_end(args);
    return ret;
}
