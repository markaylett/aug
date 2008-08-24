/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/stream.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/socket.h"
#include "augsys/uio.h"
#include "augsys/unistd.h"

#include <assert.h>

struct fimpl_ {
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_fd fd_;
};

static void*
fstream_cast_(aug_stream* ob, const char* id)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_streamid)) {
        aug_retain(&impl->stream_);
        return &impl->stream_;
    }
    return NULL;
}

static void
fstream_retain_(aug_stream* ob)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
fstream_release_(aug_stream* ob)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static aug_result
fstream_shutdown_(aug_stream* ob)
{
    return AUG_SUCCESS;
}

static ssize_t
fstream_read_(aug_stream* ob, void* buf, size_t size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_fread(impl->fd_, buf, size);
}

static ssize_t
fstream_readv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_freadv(impl->fd_, iov, size);
}

static ssize_t
fstream_write_(aug_stream* ob, const void* buf, size_t size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_fwrite(impl->fd_, buf, size);
}

static ssize_t
fstream_writev_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_fwritev(impl->fd_, iov, size);
}

static const struct aug_streamvtbl fstreamvtbl_ = {
    fstream_cast_,
    fstream_retain_,
    fstream_release_,
    fstream_shutdown_,
    fstream_read_,
    fstream_readv_,
    fstream_write_,
    fstream_writev_
};

struct simpl_ {
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_sd sd_;
};

static void*
sstream_cast_(aug_stream* ob, const char* id)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_streamid)) {
        aug_retain(&impl->stream_);
        return &impl->stream_;
    }
    return NULL;
}

static void
sstream_retain_(aug_stream* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
sstream_release_(aug_stream* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static aug_result
sstream_shutdown_(aug_stream* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_sshutdown(impl->sd_, SHUT_WR);
}

static ssize_t
sstream_read_(aug_stream* ob, void* buf, size_t size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_sread(impl->sd_, buf, size);
}

static ssize_t
sstream_readv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_sreadv(impl->sd_, iov, size);
}

static ssize_t
sstream_write_(aug_stream* ob, const void* buf, size_t size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_swrite(impl->sd_, buf, size);
}

static ssize_t
sstream_writev_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_swritev(impl->sd_, iov, size);
}

static const struct aug_streamvtbl sstreamvtbl_ = {
    sstream_cast_,
    sstream_retain_,
    sstream_release_,
    sstream_shutdown_,
    sstream_read_,
    sstream_readv_,
    sstream_write_,
    sstream_writev_
};

AUGSYS_API aug_stream*
aug_createfstream(aug_mpool* mpool, aug_fd fd)
{
    struct fimpl_* impl = aug_allocmem(mpool, sizeof(struct fimpl_));
    if (!impl)
        return NULL;

    impl->stream_.vtbl_ = &fstreamvtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->fd_ = fd;

    aug_retain(mpool);
    return &impl->stream_;
}

AUGSYS_API aug_stream*
aug_createsstream(aug_mpool* mpool, aug_sd sd)
{
    struct simpl_* impl = aug_allocmem(mpool, sizeof(struct simpl_));
    if (!impl)
        return NULL;

    impl->stream_.vtbl_ = &sstreamvtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->sd_ = sd;

    aug_retain(mpool);
    return &impl->stream_;
}
