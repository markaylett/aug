/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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

#include "augext/err.h"
#include "augext/stream.h"

#include <assert.h>
#include <string.h>

struct impl_ {
    aug_chan chan_;
    aug_stream stream_;
    aug_err err_;
    int refs_;
    aug_mpool* mpool_;
    char name_[AUG_MAXCHANNAMELEN + 1];
    unsigned id_;
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
    } else if (AUG_EQUALID(id, aug_errid)) {
        aug_retain(&impl->err_);
        return &impl->err_;
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

    if (!impl->init_) {

        /* Events will be zero on initial call.  This is used to signify
           channel establishment. */

        impl->init_ = AUG_TRUE;

        /* Id and parent-id are the same. */

        if (!aug_safeestab(ob, handler, impl->id_, &impl->stream_, impl->id_))
            return NULL;

    } else {

#if !defined(_WIN32)
        unsigned short events = aug_getmdevents(impl->muxer_, impl->fd_);
#else /* _WIN32 */
        unsigned short events = 0;
#endif /* _WIN32 */

        /* Assumption: error events cannot occur on plain files. */

        if (events && !aug_safeready(ob, handler, impl->id_, &impl->stream_,
                                     events))
            return NULL;
    }

    retain_(impl);
    return ob;
}

static aug_result
chan_setmask_(aug_chan* ob, unsigned short mask)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return aug_setmdeventmask(impl->muxer_, impl->fd_, mask);
#else /* _WIN32 */
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                   AUG_MSG("aug_setmdeventmask() not supported"));
    return AUG_FAILERROR;
#endif /* _WIN32 */
}

static unsigned short
chan_getmask_(aug_chan* ob)
{
#if !defined(_WIN32)
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return aug_getmdeventmask(impl->muxer_, impl->fd_);
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

static aug_bool
chan_isblocked_(aug_chan* ob)
{
    return AUG_TRUE;
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
    chan_getname_,
    chan_isblocked_
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

static aug_rsize
stream_read_(aug_stream* ob, void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fread(impl->fd_, buf, size);
}

static aug_rsize
stream_readv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_freadv(impl->fd_, iov, size);
}

static aug_rsize
stream_write_(aug_stream* ob, const void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return aug_fwrite(impl->fd_, buf, size);
}

static aug_rsize
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

static void*
err_cast_(aug_err* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, err_, ob);
    return cast_(impl, id);
}

static void
err_retain_(aug_err* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, err_, ob);
    retain_(impl);
}

static void
err_release_(aug_err* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, err_, ob);
    release_(impl);
}

static void
err_copyerrinfo_(aug_err* ob, struct aug_errinfo* dst)
{
    const struct aug_errinfo* src = aug_tlerr;
    aug_seterrinfo(dst, src->file_, src->line_, src->src_, src->num_,
                   src->desc_);
}

static const struct aug_errvtbl errvtbl_ = {
    err_cast_,
    err_retain_,
    err_release_,
    err_copyerrinfo_
};

AUGSYS_API void
aug_safeerror(aug_chan* chan, aug_chandler* handler, unsigned id,
              struct aug_errinfo* errinfo)
{
    aug_clearerrinfo(aug_tlerr);

    /* Lock here to prevent release during callback. */

    aug_retain(chan);
    aug_errorchan(handler, id, errinfo);
    aug_release(chan);
}

AUGSYS_API aug_bool
aug_safeestab(aug_chan* chan, aug_chandler* handler, unsigned id,
              aug_stream* ob, unsigned parent)
{
    aug_bool ret;

    /* The callback is not required to set errinfo when returning false.  The
       errinfo record must therefore be cleared before the callback is made,
       to avoid any confusion with previous errors. */

    aug_clearerrinfo(aug_tlerr);

    /* Lock here to prevent release during callback. */

    aug_retain(chan);
    ret = aug_estabchan(handler, id, ob, parent);
    aug_release(chan);

    return ret;
}

AUGSYS_API aug_bool
aug_safeready(aug_chan* chan, aug_chandler* handler, unsigned id,
              aug_stream* ob, unsigned short events)
{
    aug_bool ret;

    /* The callback is not required to set errinfo when returning false.  The
       errinfo record must therefore be cleared before the callback is made,
       to avoid any confusion with previous errors. */

    aug_clearerrinfo(aug_tlerr);

    /* Lock here to prevent release during callback. */

    aug_retain(chan);
    ret = aug_readychan(handler, id, ob, events);
    aug_release(chan);

    return ret;
}

AUGSYS_API aug_chan*
aug_createfile(aug_mpool* mpool, const char* name, aug_muxer_t muxer,
               aug_fd fd)
{
    struct impl_* impl = aug_allocmem(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    /* Now established.  Force multiplexer to return immediately so that
       establishment can be finalised in process() function. */

    aug_setmdevents(muxer, 1);

    impl->chan_.vtbl_ = &chanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->stream_.vtbl_ = &streamvtbl_;
    impl->stream_.impl_ = NULL;
    impl->err_.vtbl_ = &errvtbl_;
    impl->err_.impl_ = NULL;
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
