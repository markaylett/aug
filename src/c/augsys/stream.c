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
#define AUGSYS_BUILD
#include "augsys/stream.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/socket.h"
#include "augsys/uio.h"
#include "augsys/unistd.h"

#include <assert.h>
#include <string.h> /* strcmp() */

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
    return 0;
}

static aug_rsize
fstream_read_AI_(aug_stream* ob, void* buf, size_t size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_fread_AI(impl->fd_, buf, size);
}

static aug_rsize
fstream_readv_AI_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_freadv_AI(impl->fd_, iov, size);
}

static aug_rsize
fstream_write_AI_(aug_stream* ob, const void* buf, size_t size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_fwrite_AI(impl->fd_, buf, size);
}

static aug_rsize
fstream_writev_AI_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct fimpl_* impl = AUG_PODIMPL(struct fimpl_, stream_, ob);
    return aug_fwritev_AI(impl->fd_, iov, size);
}

static const struct aug_streamvtbl fstreamvtbl_ = {
    fstream_cast_,
    fstream_retain_,
    fstream_release_,
    fstream_shutdown_,
    fstream_read_AI_,
    fstream_readv_AI_,
    fstream_write_AI_,
    fstream_writev_AI_
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

static aug_rsize
sstream_read_AI_(aug_stream* ob, void* buf, size_t size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_sread_AI(impl->sd_, buf, size);
}

static aug_rsize
sstream_readv_AI_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_sreadv_AI(impl->sd_, iov, size);
}

static aug_rsize
sstream_write_AI_(aug_stream* ob, const void* buf, size_t size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_swrite_AI(impl->sd_, buf, size);
}

static aug_rsize
sstream_writev_AI_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, stream_, ob);
    return aug_swritev_AI(impl->sd_, iov, size);
}

static const struct aug_streamvtbl sstreamvtbl_ = {
    sstream_cast_,
    sstream_retain_,
    sstream_release_,
    sstream_shutdown_,
    sstream_read_AI_,
    sstream_readv_AI_,
    sstream_write_AI_,
    sstream_writev_AI_
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
