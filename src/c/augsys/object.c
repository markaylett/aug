/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/base.h" /* aug_nextid() */
#include "augsys/uio.h"
#include "augsys/unistd.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/stream.h"

#include <assert.h>
#include <string.h>

struct impl_ {
    aug_chan chan_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    unsigned id_;
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
chan_process_(aug_chan* ob, aug_bool* fork, aug_chancb_t cb, aug_object* cbob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    int events;

#if !defined(_WIN32)
    events = aug_fdevents(impl->muxer_, impl->fd_);
#else /* _WIN32 */
    events = 0;
#endif /* _WIN32 */

    /* The callback is not required to set errinfo when returning false.  The
       errinfo record must therefore be cleared before the callback is made to
       avoid any confusion with previous errors. */

    aug_clearerrinfo(aug_tlerr);

    /* Lock here to prevent release during callback. */

    retain_(impl);

    if (events < 0 || !cb(cbob, impl->id_, &impl->stream_, events)) {
        release_(impl);
        return NULL;
    }

    return ob;
}

static aug_result
chan_setmask_(aug_chan* ob, unsigned short mask)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return aug_setfdeventmask(impl->muxer_, impl->fd_, mask);
#else /* _WIN32 */
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                   AUG_MSG("aug_setfdeventmask() not supported"));
    return AUG_FAILERROR;
#endif /* _WIN32 */
}

static int
chan_getmask_(aug_chan* ob)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return aug_fdeventmask(impl->muxer_, impl->fd_);
#else /* _WIN32 */
    return 0;
#endif /* _WIN32 */
}

static unsigned
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

static const struct aug_chanvtbl chanvtbl_ = {
    chan_cast_,
    chan_retain_,
    chan_release_,
    chan_close_,
    chan_process_,
    chan_setmask_,
    chan_getmask_,
    chan_getid_,
    chan_getname_
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
    return AUG_SUCCESS;
}

static ssize_t
stream_read_(aug_stream* ob, void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fread(impl->fd_, buf, size);
}

static ssize_t
stream_readv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_freadv(impl->fd_, iov, size);
}

static ssize_t
stream_write_(aug_stream* ob, const void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fwrite(impl->fd_, buf, size);
}

static ssize_t
stream_writev_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fwritev(impl->fd_, iov, size);
}

static const struct aug_streamvtbl streamvtbl_ = {
    stream_cast_,
    stream_retain_,
    stream_release_,
    stream_shutdown_,
    stream_read_,
    stream_readv_,
    stream_write_,
    stream_writev_
};

AUGSYS_API aug_chan*
aug_createfile(aug_mpool* mpool, aug_fd fd, aug_muxer_t muxer)
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
    impl->id_ = aug_nextid();
    impl->fd_ = fd;
    impl->muxer_ = muxer;

    aug_retain(mpool);
    return &impl->chan_;
}
