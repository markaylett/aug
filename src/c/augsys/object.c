/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/uio.h"
#include "augsys/unistd.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augob/streamob.h"

#include <assert.h>
#include <string.h>

struct impl_ {
    aug_channelob channelob_;
    aug_streamob streamob_;
    int refs_;
    aug_mpool* mpool_;
    aug_fd fd_;
    aug_muxer_t muxer_;
};

static aug_result
close_(struct impl_* impl)
{
#if !defined(_WIN32)
    aug_setfdeventmask(impl->muxer_, impl->fd_, 0);
#endif /* !_WIN32 */
    return aug_fclose(impl->fd_);
}

static void*
cast_(struct impl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_channelobid)) {
        aug_retain(&impl->channelob_);
        return &impl->channelob_;
    } else if (AUG_EQUALID(id, aug_streamobid)) {
        aug_retain(&impl->streamob_);
        return &impl->streamob_;
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
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static void*
channelob_cast_(aug_channelob* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    return cast_(impl, id);
}

static void
channelob_retain_(aug_channelob* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    retain_(impl);
}

static void
channelob_release_(aug_channelob* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    release_(impl);
}

static aug_result
channelob_close_(aug_channelob* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    aug_result result = close_(impl);
    impl->fd_ = AUG_BADFD;
    return result;
}

static aug_channelob*
channelob_process_(aug_channelob* ob, aug_channelcb_t cb, aug_bool* fork)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    int events;

#if !defined(_WIN32)
    events = aug_fdevents(impl->muxer_, impl->fd_);
#else /* _WIN32 */
    events = 0;
#endif /* _WIN32 */

    /* Lock here to prevent release during callback. */

    retain_(impl);

    if (events < 0 || !cb(&impl->streamob_, events)) {
        release_(impl);
        return NULL;
    }

    return ob;
}

static aug_result
channelob_setnonblock_(aug_channelob* ob, aug_bool on)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    return aug_fsetnonblock(impl->fd_, on);
}

static aug_result
channelob_seteventmask_(aug_channelob* ob, unsigned short mask)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    return aug_setfdeventmask(impl->muxer_, impl->fd_, mask);
#else /* _WIN32 */
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                   AUG_MSG("aug_setfdeventmask() not supported"));
    return AUG_FAILERROR;
#endif /* _WIN32 */
}

static int
channelob_eventmask_(aug_channelob* ob)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    return aug_fdeventmask(impl->muxer_, impl->fd_);
#else /* _WIN32 */
    return 0;
#endif /* _WIN32 */
}

static int
channelob_events_(aug_channelob* ob)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    return aug_fdevents(impl->muxer_, impl->fd_);
#else /* _WIN32 */
    return 0;
#endif /* _WIN32 */
}

static const struct aug_channelobvtbl channelobvtbl_ = {
    channelob_cast_,
    channelob_retain_,
    channelob_release_,
    channelob_close_,
    channelob_process_,
    channelob_setnonblock_,
    channelob_seteventmask_,
    channelob_eventmask_,
    channelob_events_
};

static void*
streamob_cast_(aug_streamob* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, streamob_, ob);
    return cast_(impl, id);
}

static void
streamob_retain_(aug_streamob* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, streamob_, ob);
    retain_(impl);
}

static void
streamob_release_(aug_streamob* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, streamob_, ob);
    release_(impl);
}

static aug_result
streamob_shutdown_(aug_streamob* ob)
{
    return AUG_SUCCESS;
}

static ssize_t
streamob_read_(aug_streamob* ob, void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, streamob_, ob);
    return aug_fread(impl->fd_, buf, size);
}

static ssize_t
streamob_readv_(aug_streamob* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, streamob_, ob);
    return aug_freadv(impl->fd_, iov, size);
}

static ssize_t
streamob_write_(aug_streamob* ob, const void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, streamob_, ob);
    return aug_fwrite(impl->fd_, buf, size);
}

static ssize_t
streamob_writev_(aug_streamob* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, streamob_, ob);
    return aug_fwritev(impl->fd_, iov, size);
}

static const struct aug_streamobvtbl streamobvtbl_ = {
    streamob_cast_,
    streamob_retain_,
    streamob_release_,
    streamob_shutdown_,
    streamob_read_,
    streamob_readv_,
    streamob_write_,
    streamob_writev_
};

AUGSYS_API aug_channelob*
aug_createfile(aug_mpool* mpool, aug_fd fd, aug_muxer_t muxer)
{
    struct impl_* impl = aug_malloc(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->channelob_.vtbl_ = &channelobvtbl_;
    impl->channelob_.impl_ = NULL;
    impl->streamob_.vtbl_ = &streamobvtbl_;
    impl->streamob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->fd_ = fd;
    impl->muxer_ = muxer;

    aug_retain(mpool);
    return &impl->channelob_;
}
