/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/defs.h" /* AUG_MAXLINE */
#include "augctx/errinfo.h"

#include "augext/log.h"

#include <assert.h>
#include <stdio.h>       /* _vsnprintf() */

#define AUG_MSGID 1

static void
report_(HANDLE handle, WORD type, const char* format, va_list args)
{
    char msg[AUG_MAXLINE];
    const char* msgs[1];
    msgs[0] = msg;

    /* Null termination is _not_ guaranteed by snprintf(). */

    _vsnprintf(msg, sizeof(msg) - 1, format, args);
    msg[sizeof(msg) - 1] = '\0';

    ReportEvent(handle, type, 0, AUG_MSGID, NULL, 1, 0,
                (const char**)&msgs[0], NULL);
}

struct impl_ {
    aug_log log_;
    int refs_;
    aug_mpool* mpool_;
    aug_log* inner_;
    HANDLE handle_;
};

static void*
cast_(aug_log* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_logid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_log* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_log* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        DeregisterEventSource(impl->handle_);
        aug_release(impl->inner_);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static aug_result
vwritelog_(aug_log* obj, int level, const char* format, va_list args)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, obj);
    switch (level) {
    case AUG_LOGCRIT:
    case AUG_LOGERROR:
        report_(impl->handle_, EVENTLOG_ERROR_TYPE, format, args);
        break;
    case AUG_LOGWARN:
        report_(impl->handle_, EVENTLOG_WARNING_TYPE, format, args);
        break;
    case AUG_LOGNOTICE:
        report_(impl->handle_, EVENTLOG_INFORMATION_TYPE, format, args);
        break;
    }
    return aug_vwritelog(impl->inner_, level, format, args);
}

static const struct aug_logvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    vwritelog_
};

static aug_log*
createsrvlog_(aug_mpool* mpool, const char* sname, aug_log* inner)
{
    struct impl_* impl;
    HANDLE handle;
    assert(mpool);
    assert(inner);

    if (!(impl = aug_allocmem(mpool, sizeof(struct impl_))))
        return NULL;

    if (!(handle = RegisterEventSource(NULL, sname))) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        aug_freemem(mpool, impl);
        return NULL;
    }

    impl->log_.vtbl_ = &vtbl_;
    impl->log_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->inner_ = inner;
    impl->handle_ = handle;

    aug_retain(mpool);
    aug_retain(inner);
    return &impl->log_;
}

AUGSRV_API aug_result
aug_setsrvlogger(const char* sname)
{
    aug_mpool* mpool = aug_getmpool(aug_tlx);
    aug_log* inner = aug_getlog(aug_tlx);
    aug_log* outer = createsrvlog_(mpool, sname, inner);
    if (!outer)
        return AUG_FAILERROR;

    aug_setlog(aug_tlx, outer);
    aug_release(outer);
    aug_release(inner);
    aug_release(mpool);
    return AUG_SUCCESS;
}
