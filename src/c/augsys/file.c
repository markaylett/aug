/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/file.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/socket.h"
#include "augsys/stream.h"
#include "augsys/uio.h"
#include "augsys/unistd.h"

#include "augctx/base.h"

#include <assert.h>
#include <string.h>

struct fimpl_ {
    aug_file file_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_fd fd_;
};

static void*
fcast_(struct fimpl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_fileid)) {
        aug_retain(&impl->file_);
        return &impl->file_;
    } else if (AUG_EQUALID(id, aug_streamid)) {
        aug_retain(&impl->stream_);
        return &impl->stream_;
    }
    return NULL;
}

static void
fretain_(struct fimpl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
frelease_(struct fimpl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (AUG_BADFD != impl->fd_)
            aug_fclose(impl->fd_);
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static void*
file_fcast_(aug_file* obj, const char* id)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, file_, obj);
    return fcast_(impl, id);
}

static void
file_fretain_(aug_file* obj)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, file_, obj);
    fretain_(impl);
}

static void
file_frelease_(aug_file* obj)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, file_, obj);
    frelease_(impl);
}

static aug_result
file_fclose_(aug_file* obj)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, file_, obj);
    aug_result result = aug_fclose(impl->fd_);
    impl->fd_ = AUG_BADFD;
    return result;
}

static aug_result
file_fsetnonblock_(aug_file* obj, aug_bool on)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, file_, obj);
    return aug_fsetnonblock(impl->fd_, on);
}

static const struct aug_filevtbl file_fvtbl_ = {
    file_fcast_,
    file_fretain_,
    file_frelease_,
    file_fclose_,
    file_fsetnonblock_
};

static void*
stream_fcast_(aug_stream* obj, const char* id)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, obj);
    return fcast_(impl, id);
}

static void
stream_fretain_(aug_stream* obj)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, obj);
    fretain_(impl);
}

static void
stream_frelease_(aug_stream* obj)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, obj);
    frelease_(impl);
}

static ssize_t
stream_fread_(aug_stream* obj, void* buf, size_t size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, obj);
    return aug_fread(impl->fd_, buf, size);
}

static ssize_t
stream_freadv_(aug_stream* obj, const struct iovec* iov, int size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, obj);
    return aug_freadv(impl->fd_, iov, size);
}

static ssize_t
stream_fwrite_(aug_stream* obj, const void* buf, size_t size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, obj);
    return aug_fwrite(impl->fd_, buf, size);
}

static ssize_t
stream_fwritev_(aug_stream* obj, const struct iovec* iov, int size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, obj);
    return aug_fwritev(impl->fd_, iov, size);
}

static const struct aug_streamvtbl stream_fvtbl_ = {
    stream_fcast_,
    stream_fretain_,
    stream_frelease_,
    stream_fread_,
    stream_freadv_,
    stream_fwrite_,
    stream_fwritev_
};

AUGSYS_API aug_file*
aug_attachfd(aug_mpool* mpool, aug_fd fd)
{
    struct fimpl_* impl = aug_malloc(mpool, sizeof(struct fimpl_));
    if (!impl)
        return NULL;

    impl->file_.vtbl_ = &file_fvtbl_;
    impl->file_.impl_ = NULL;
    impl->stream_.vtbl_ = &stream_fvtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->fd_ = fd;

    aug_retain(mpool);
    return &impl->file_;
}

struct simpl_ {
    aug_file file_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_sd sd_;
};

static void*
scast_(struct simpl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_fileid)) {
        aug_retain(&impl->file_);
        return &impl->file_;
    } else if (AUG_EQUALID(id, aug_streamid)) {
        aug_retain(&impl->stream_);
        return &impl->stream_;
    }
    return NULL;
}

static void
sretain_(struct simpl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
srelease_(struct simpl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (AUG_BADSD != impl->sd_)
            aug_sclose(impl->sd_);
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static void*
file_scast_(aug_file* obj, const char* id)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, file_, obj);
    return scast_(impl, id);
}

static void
file_sretain_(aug_file* obj)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, file_, obj);
    sretain_(impl);
}

static void
file_srelease_(aug_file* obj)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, file_, obj);
    srelease_(impl);
}

static aug_result
file_sclose_(aug_file* obj)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, file_, obj);
    aug_result result = aug_sclose(impl->sd_);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_result
file_ssetnonblock_(aug_file* obj, aug_bool on)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, file_, obj);
    return aug_ssetnonblock(impl->sd_, on);
}

static const struct aug_filevtbl file_svtbl_ = {
    file_scast_,
    file_sretain_,
    file_srelease_,
    file_sclose_,
    file_ssetnonblock_
};

static void*
stream_scast_(aug_stream* obj, const char* id)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, obj);
    return scast_(impl, id);
}

static void
stream_sretain_(aug_stream* obj)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, obj);
    sretain_(impl);
}

static void
stream_srelease_(aug_stream* obj)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, obj);
    srelease_(impl);
}

static ssize_t
stream_sread_(aug_stream* obj, void* buf, size_t size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, obj);
    return aug_sread(impl->sd_, buf, size);
}

static ssize_t
stream_sreadv_(aug_stream* obj, const struct iovec* iov, int size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, obj);
    return aug_sreadv(impl->sd_, iov, size);
}

static ssize_t
stream_swrite_(aug_stream* obj, const void* buf, size_t size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, obj);
    return aug_swrite(impl->sd_, buf, size);
}

static ssize_t
stream_swritev_(aug_stream* obj, const struct iovec* iov, int size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, obj);
    return aug_swritev(impl->sd_, iov, size);
}

static const struct aug_streamvtbl stream_svtbl_ = {
    stream_scast_,
    stream_sretain_,
    stream_srelease_,
    stream_sread_,
    stream_sreadv_,
    stream_swrite_,
    stream_swritev_
};

AUGSYS_API aug_file*
aug_attachsd(aug_mpool* mpool, aug_sd sd)
{
    struct simpl_* impl = aug_malloc(mpool, sizeof(struct simpl_));
    if (!impl)
        return NULL;

    impl->file_.vtbl_ = &file_svtbl_;
    impl->file_.impl_ = NULL;
    impl->stream_.vtbl_ = &stream_svtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->sd_ = sd;

    aug_retain(mpool);
    return &impl->file_;
}
