/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/stream.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"

#include <assert.h>
#include <string.h>

struct impl_ {
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
};

static void*
cast_(aug_stream* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_streamid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_stream* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_stream* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static aug_result
shutdown_(aug_stream* obj)
{
    return AUG_SUCCESS;
}

static ssize_t
read_(aug_stream* obj, void* buf, size_t size)
{
    return 0;
}

static ssize_t
readv_(aug_stream* obj, const struct iovec* iov, int size)
{
    return 0;
}

static ssize_t
write_(aug_stream* obj, const void* buf, size_t size)
{
    return 0;
}

static ssize_t
writev_(aug_stream* obj, const struct iovec* iov, int size)
{
    return 0;
}

static const struct aug_streamvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    shutdown_,
    read_,
    readv_,
    write_,
    writev_
};

AUGSYS_API aug_stream*
aug_createstream(aug_mpool* mpool)
{
    struct impl_* impl = aug_malloc(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->stream_.vtbl_ = &vtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    aug_retain(mpool);
    return &impl->stream_;
}
