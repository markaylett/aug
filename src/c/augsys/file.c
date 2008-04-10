/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/file.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/stream.h"
#include "augsys/uio.h"
#include "augsys/unistd.h"

#include "augctx/base.h"

#include <assert.h>
#include <string.h>

struct impl_ {
    aug_file file_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_fd fd_;
};

static void*
cast_(struct impl_* impl, const char* id)
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
retain_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(struct impl_* impl)
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
file_cast_(aug_file* obj, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    return cast_(impl, id);
}

static void
file_retain_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    retain_(impl);
}

static void
file_release_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    release_(impl);
}

static aug_result
file_close_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    aug_result result = aug_fclose(impl->fd_);
    impl->fd_ = AUG_BADFD;
    return result;
}

static aug_result
file_setnonblock_(aug_file* obj, aug_bool on)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    return aug_fsetnonblock(impl->fd_, on);
}

static aug_fd
file_getfd_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    return impl->fd_;
}

static const struct aug_filevtbl file_vtbl_ = {
    file_cast_,
    file_retain_,
    file_release_,
    file_close_,
    file_setnonblock_,
    file_getfd_
};

static void*
stream_cast_(aug_stream* obj, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    return cast_(impl, id);
}

static void
stream_retain_(aug_stream* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    retain_(impl);
}

static void
stream_release_(aug_stream* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    release_(impl);
}

static ssize_t
stream_read_(aug_stream* obj, void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    return aug_fread(impl->fd_, buf, size);
}

static ssize_t
stream_readv_(aug_stream* obj, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    return aug_freadv(impl->fd_, iov, size);
}

static ssize_t
stream_write_(aug_stream* obj, const void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    return aug_fwrite(impl->fd_, buf, size);
}

static ssize_t
stream_writev_(aug_stream* obj, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, obj);
    return aug_fwritev(impl->fd_, iov, size);
}

static const struct aug_streamvtbl stream_vtbl_ = {
    stream_cast_,
    stream_retain_,
    stream_release_,
    stream_read_,
    stream_readv_,
    stream_write_,
    stream_writev_
};

static aug_file*
vcreatefile_(const char* path, int flags, va_list args)
{
    aug_fd fd;
    aug_mpool* mpool = aug_getmpool(aug_tlx);
    struct impl_* impl = aug_malloc(mpool, sizeof(struct impl_));

    if (!impl) {
        aug_release(mpool);
        return NULL;
    }

    if (AUG_BADFD == (fd = aug_vfopen(path, flags, args))) {
        aug_free(mpool, impl);
        aug_release(mpool);
        return NULL;
    }

    impl->file_.vtbl_ = &file_vtbl_;
    impl->file_.impl_ = NULL;
    impl->stream_.vtbl_ = &stream_vtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->fd_ = fd;

    return &impl->file_;
}

AUGSYS_API aug_file*
aug_createfile(const char* path, int flags, ...)
{
    aug_file* file;
    va_list args;
    va_start(args, flags);
    file = vcreatefile_(path, flags, args);
    va_end(args);
    return file;
}
