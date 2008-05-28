/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnet.h"
#include "augctx.h"

#include <assert.h>
#include <stdio.h>

static int refs_ = 0;

struct impl_ {
    aug_channelob channelob_;
    aug_streamob streamob_;
    int refs_;
    aug_mpool* mpool_;
    unsigned id_;
};

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
    ++refs_;
}

static void
release_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
    --refs_;
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
    return AUG_SUCCESS;
}

static aug_channelob*
channelob_process_(aug_channelob* ob, aug_bool* fork, aug_channelcb_t cb,
                   aug_object* cbob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);

    /* Lock here to prevent release during callback. */

    retain_(impl);

    if (!cb(cbob, impl->id_, &impl->streamob_, 0)) {
        release_(impl);
        return NULL;
    }

    return ob;
}

static aug_result
channelob_seteventmask_(aug_channelob* ob, unsigned short mask)
{
    return AUG_SUCCESS;
}

static unsigned
channelob_getid_(aug_channelob* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, channelob_, ob);
    return impl->id_;
}

static int
channelob_eventmask_(aug_channelob* ob)
{
    return 0;
}

static const struct aug_channelobvtbl channelobvtbl_ = {
    channelob_cast_,
    channelob_retain_,
    channelob_release_,
    channelob_close_,
    channelob_process_,
    channelob_seteventmask_,
    channelob_getid_,
    channelob_eventmask_
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
    return 0;
}

static ssize_t
streamob_readv_(aug_streamob* ob, const struct iovec* iov, int size)
{
    return 0;
}

static ssize_t
streamob_write_(aug_streamob* ob, const void* buf, size_t size)
{
    return 0;
}

static ssize_t
streamob_writev_(aug_streamob* ob, const struct iovec* iov, int size)
{
    return 0;
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

static aug_channelob*
create_(aug_mpool* mpool, unsigned id)
{
    struct impl_* impl = aug_allocmem(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->channelob_.vtbl_ = &channelobvtbl_;
    impl->channelob_.impl_ = NULL;
    impl->streamob_.vtbl_ = &streamobvtbl_;
    impl->streamob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->id_ = id;

    ++refs_;

    aug_retain(mpool);
    return &impl->channelob_;
}

static int count_ = 0;
static unsigned last_ = 0;

static aug_bool
cb_(aug_object* cbob, unsigned id, aug_streamob* streamob,
    unsigned short events)
{
    if (3 < id)
        aug_die("invalid channel");
    ++count_;
    last_ = id;
    return AUG_TRUE;
}

static void
foreach_(aug_channels_t channels)
{
    count_ = 0;
    last_ = 0;
    aug_foreachchannel(channels, cb_, NULL);
}

static aug_bool
rm1_(aug_object* cbob, unsigned id, aug_streamob* streamob,
     unsigned short events)
{
    return 1 == id ? AUG_FALSE : AUG_TRUE;
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_channelob* channelob1;
    aug_channelob* channelob2;
    aug_channelob* channelob3;
    aug_channels_t channels;

    aug_check(0 <= aug_atbasixtlx());
    aug_setloglevel(aug_tlx, AUG_LOGDEBUG0 + 3);

    mpool = aug_getmpool(aug_tlx);
    aug_check(mpool);

    channelob1 = create_(mpool, 1);
    aug_check(channelob1);

    channelob2 = create_(mpool, 2);
    aug_check(channelob2);

    channelob3 = create_(mpool, 3);
    aug_check(channelob3);

    channels = aug_createchannels(mpool);
    aug_check(channels);

    aug_check(0 <= aug_insertchannel(channels, channelob1));
    aug_check(0 <= aug_insertchannel(channels, channelob2));
    aug_check(0 <= aug_insertchannel(channels, channelob3));
    aug_check(6 == refs_);

    aug_release(channelob1);
    aug_release(channelob2);
    aug_release(channelob3);
    aug_check(3 == refs_);

    foreach_(channels);
    aug_check(3 == aug_getchannels(channels));
    aug_check(3 == count_);
    aug_check(3 == last_);

    /* Fairness rotation. */

    foreach_(channels);
    aug_check(3 == aug_getchannels(channels));
    aug_check(3 == count_);
    aug_check(1 == last_);

    /* Fairness rotation. */

    foreach_(channels);
    aug_check(3 == aug_getchannels(channels));
    aug_check(3 == count_);
    aug_check(2 == last_);

    /* Remove middle. */

    aug_check(0 <= aug_removechannel(channels, 2));

    foreach_(channels);
    aug_check(2 == aug_getchannels(channels));
    aug_check(2 == count_);
    aug_check(3 == last_);

    /* No longer exists. */

    aug_check(AUG_FAILNONE == aug_removechannel(channels, 2));

    /* Remove during loop. */

    aug_foreachchannel(channels, rm1_, NULL);

    /* No longer exists. */

    aug_check(AUG_FAILNONE == aug_removechannel(channels, 1));

    foreach_(channels);
    aug_check(1 == aug_getchannels(channels));
    aug_check(1 == count_);
    aug_check(3 == last_);

    aug_destroychannels(channels);

     /* Objects released. */

    aug_check(0 == aug_getchannels(channels));
    aug_check(0 == refs_);
    aug_release(mpool);
    return 0;
}
