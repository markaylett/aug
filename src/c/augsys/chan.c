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
#include "augsys/chan.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/uio.h"
#include "augsys/unistd.h"
#include "augsys/utility.h" /* aug_nextid() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"  /* aug_strlcpy() */

#include "augext/stream.h"

#include <assert.h>
#include <string.h>

struct impl_ {
    aug_chan chan_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    char name_[AUG_MAXCHANNAMELEN + 1];
    aug_id id_;
    aug_muxer_t muxer_;
    aug_fd fd_;
    aug_bool init_;
};

static aug_result
close_(struct impl_* impl)
{
#if !defined(_WIN32)
    aug_setmdeventmask(impl->muxer_, impl->fd_, 0);
#endif /* !_WIN32 */
    return aug_fclose(impl->fd_);
}

static void*
cast_(struct impl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_chanid)) {
        aug_retain(&impl->chan_);
        return &impl->chan_;
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
            close_(impl);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
chan_cast_(aug_chan* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return cast_(impl, id);
}

static void
chan_retain_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    retain_(impl);
}

static void
chan_release_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    release_(impl);
}

static aug_result
chan_close_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    aug_result result = close_(impl);
    impl->fd_ = AUG_BADFD;
    return result;
}

static aug_chan*
chan_process_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);

    /* Channel closed. */

    if (AUG_BADFD == impl->fd_)
        return NULL;

    if (!impl->init_) {

        /* Events will be zero on initial call.  This is used to signify
           channel establishment. */

        impl->init_ = AUG_TRUE;

        /* In this case, id and parent-id are the same. */

        if (!aug_estabchan(handler, &impl->chan_, impl->id_))
            return NULL;

    } else {

#if !defined(_WIN32)
        unsigned short events = aug_getmdevents(impl->muxer_, impl->fd_);
#else /* _WIN32 */
        unsigned short events = 0;
#endif /* _WIN32 */

        /* Assumption: error events cannot occur on plain files. */

        if (events && !aug_readychan(handler, &impl->chan_, events))
            return NULL;
    }

    retain_(impl);
    return ob;
}

static aug_result
chan_setwantwr_(aug_chan* ob, aug_bool wantwr)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return aug_setmdeventmask(impl->muxer_, impl->fd_,
                              wantwr ? AUG_MDEVENTALL : AUG_MDEVENTRDEX);
#else /* _WIN32 */
    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                    AUG_MSG("aug_setmdeventmask() not supported"));
    return -1;
#endif /* _WIN32 */
}

static aug_id
chan_getid_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return impl->id_;
}

static char*
chan_getname_(aug_chan* ob, char* dst, unsigned size)
{
    strcpy(dst, "test");
    return dst;
}

static aug_bool
chan_isready_(aug_chan* ob)
{
    /* If not initialised, then force processing so that establishment can be
       finalised in process() function. */

    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);

    /* True if closed. */

    if (AUG_BADFD == impl->fd_)
        return AUG_TRUE;

    return !impl->init_;
}

static const struct aug_chanvtbl chanvtbl_ = {
    chan_cast_,
    chan_retain_,
    chan_release_,
    chan_close_,
    chan_process_,
    chan_setwantwr_,
    chan_getid_,
    chan_getname_,
    chan_isready_
};

static void*
stream_cast_(aug_stream* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return cast_(impl, id);
}

static void
stream_retain_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    retain_(impl);
}

static void
stream_release_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    release_(impl);
}

static aug_result
stream_shutdown_(aug_stream* ob)
{
    return 0;
}

static aug_rsize
stream_read_BI_(aug_stream* ob, void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fread_BI(impl->fd_, buf, size);
}

static aug_rsize
stream_readv_BI_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_freadv_BI(impl->fd_, iov, size);
}

static aug_rsize
stream_write_BI_(aug_stream* ob, const void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fwrite_BI(impl->fd_, buf, size);
}

static aug_rsize
stream_writev_BI_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fwritev_BI(impl->fd_, iov, size);
}

static const struct aug_streamvtbl streamvtbl_ = {
    stream_cast_,
    stream_retain_,
    stream_release_,
    stream_shutdown_,
    stream_read_BI_,
    stream_readv_BI_,
    stream_write_BI_,
    stream_writev_BI_
};

AUGSYS_API aug_chan*
aug_createfile(aug_mpool* mpool, const char* name, aug_muxer_t muxer,
               aug_fd fd)
{
    struct impl_* impl = aug_allocmem(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->chan_.vtbl_ = &chanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->stream_.vtbl_ = &streamvtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    aug_strlcpy(impl->name_, name, sizeof(impl->name_));
    impl->id_ = aug_nextid();
    impl->muxer_ = muxer;
    impl->fd_ = fd;
    impl->init_ = AUG_FALSE;

    aug_retain(mpool);
    return &impl->chan_;
}
