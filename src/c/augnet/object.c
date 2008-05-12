/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/tcpclient.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augob/streamob.h"

#include <assert.h>
#include <string.h>

struct cimpl_ {
    aug_channelob channelob_;
    int refs_;
    aug_mpool* mpool_;
    aug_tcpclient_t client_;
    aug_sd sd_;
    aug_muxer_t muxer_;
};

static aug_result
cclose_(struct cimpl_* impl)
{
    aug_setfdeventmask(impl->muxer_, impl->sd_, 0);
    return aug_sclose(impl->sd_);
}

static void*
ccast_(struct cimpl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_channelobid)) {
        aug_retain(&impl->channelob_);
        return &impl->channelob_;
    }
    return NULL;
}

static void
cretain_(struct cimpl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
crelease_(struct cimpl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (AUG_BADSD != impl->sd_)
            cclose_(impl);
        aug_destroytcpclient(impl->client_);
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static void*
cchannelob_cast_(aug_channelob* ob, const char* id)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    return ccast_(impl, id);
}

static void
cchannelob_retain_(aug_channelob* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    cretain_(impl);
}

static void
cchannelob_release_(aug_channelob* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    crelease_(impl);
}

static aug_result
cchannelob_close_(aug_channelob* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    aug_result result = cclose_(impl);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_channelob*
cchannelob_process_(aug_channelob* ob, aug_channelcb_t cb, aug_bool* fork)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);

    if ((AUG_FDEVENTRD & aug_fdevents(impl->muxer_, impl->sd_))) {

        aug_sd sd;
        struct aug_endpoint ep;
        int est;

        if (aug_setfdeventmask(impl->muxer_, impl->sd_, 0) < 0
            || AUG_BADSD == (sd = aug_tryconnect(impl->client_, &ep, &est))
            || aug_setfdeventmask(impl->muxer_, sd, AUG_FDEVENTRD) < 0)
            return NULL;

        if (est)
            return aug_createplain(impl->mpool_, sd, impl->muxer_);

        impl->sd_ = sd;
    }

    cretain_(impl);
    return ob;
}

static aug_result
cchannelob_setnonblock_(aug_channelob* ob, aug_bool on)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    return aug_ssetnonblock(impl->sd_, on);
}

static aug_result
cchannelob_seteventmask_(aug_channelob* ob, unsigned short mask)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    return aug_setfdeventmask(impl->muxer_, impl->sd_, mask);
}

static int
cchannelob_eventmask_(aug_channelob* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    return aug_fdeventmask(impl->muxer_, impl->sd_);
}

static int
cchannelob_events_(aug_channelob* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    return aug_fdevents(impl->muxer_, impl->sd_);
}

static const struct aug_channelobvtbl cchannelobvtbl_ = {
    cchannelob_cast_,
    cchannelob_retain_,
    cchannelob_release_,
    cchannelob_close_,
    cchannelob_process_,
    cchannelob_setnonblock_,
    cchannelob_seteventmask_,
    cchannelob_eventmask_,
    cchannelob_events_
};

struct pimpl_ {
    aug_channelob channelob_;
    aug_streamob streamob_;
    int refs_;
    aug_mpool* mpool_;
    aug_sd sd_;
    aug_muxer_t muxer_;
};

static aug_result
pclose_(struct pimpl_* impl)
{
    aug_setfdeventmask(impl->muxer_, impl->sd_, 0);
    return aug_sclose(impl->sd_);
}

static void*
pcast_(struct pimpl_* impl, const char* id)
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
pretain_(struct pimpl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
prelease_(struct pimpl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (AUG_BADSD != impl->sd_)
            pclose_(impl);
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static void*
pchannelob_cast_(aug_channelob* ob, const char* id)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return pcast_(impl, id);
}

static void
pchannelob_retain_(aug_channelob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    pretain_(impl);
}

static void
pchannelob_release_(aug_channelob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    prelease_(impl);
}

static aug_result
pchannelob_close_(aug_channelob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    aug_result result = pclose_(impl);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_channelob*
pchannelob_process_(aug_channelob* ob, aug_channelcb_t cb, aug_bool* fork)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    int events = aug_fdevents(impl->muxer_, impl->sd_);

    /* Lock here to prevent release during callback. */

    pretain_(impl);

    if (events < 0 || !cb(&impl->streamob_, events)) {
        prelease_(impl);
        return NULL;
    }

    return ob;
}

static aug_result
pchannelob_setnonblock_(aug_channelob* ob, aug_bool on)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return aug_ssetnonblock(impl->sd_, on);
}

static aug_result
pchannelob_seteventmask_(aug_channelob* ob, unsigned short mask)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return aug_setfdeventmask(impl->muxer_, impl->sd_, mask);
}

static int
pchannelob_eventmask_(aug_channelob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return aug_fdeventmask(impl->muxer_, impl->sd_);
}

static int
pchannelob_events_(aug_channelob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return aug_fdevents(impl->muxer_, impl->sd_);
}

static const struct aug_channelobvtbl pchannelobvtbl_ = {
    pchannelob_cast_,
    pchannelob_retain_,
    pchannelob_release_,
    pchannelob_close_,
    pchannelob_process_,
    pchannelob_setnonblock_,
    pchannelob_seteventmask_,
    pchannelob_eventmask_,
    pchannelob_events_
};

static void*
pstreamob_cast_(aug_streamob* ob, const char* id)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    return pcast_(impl, id);
}

static void
pstreamob_retain_(aug_streamob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    pretain_(impl);
}

static void
pstreamob_release_(aug_streamob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    prelease_(impl);
}

static aug_result
pstreamob_shutdown_(aug_streamob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    return aug_sshutdown(impl->sd_, SHUT_WR);
}

static ssize_t
pstreamob_read_(aug_streamob* ob, void* buf, size_t size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    return aug_sread(impl->sd_, buf, size);
}

static ssize_t
pstreamob_readv_(aug_streamob* ob, const struct iovec* iov, int size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    return aug_sreadv(impl->sd_, iov, size);
}

static ssize_t
pstreamob_write_(aug_streamob* ob, const void* buf, size_t size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    return aug_swrite(impl->sd_, buf, size);
}

static ssize_t
pstreamob_writev_(aug_streamob* ob, const struct iovec* iov, int size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, streamob_, ob);
    return aug_swritev(impl->sd_, iov, size);
}

static const struct aug_streamobvtbl pstreamobvtbl_ = {
    pstreamob_cast_,
    pstreamob_retain_,
    pstreamob_release_,
    pstreamob_shutdown_,
    pstreamob_read_,
    pstreamob_readv_,
    pstreamob_write_,
    pstreamob_writev_
};

AUGNET_API aug_channelob*
aug_createclient(aug_mpool* mpool, const char* host, const char* serv,
                 aug_muxer_t muxer)
{
    aug_tcpclient_t client;
    aug_sd sd;
    struct aug_endpoint ep;
    int est;
    struct cimpl_* impl;

    if (!(client = aug_createtcpclient(host, serv)))
        return NULL;

    if (AUG_BADSD == (sd = aug_tryconnect(client, &ep, &est)))
        goto fail1;

    if (est) {
        aug_destroytcpclient(client);
        return aug_createplain(mpool, sd, muxer);
    }

    if (aug_setfdeventmask(muxer, sd, AUG_FDEVENTRD) < 0)
        goto fail1;

    if (!(impl = aug_malloc(mpool, sizeof(struct cimpl_))))
        goto fail2;

    impl->channelob_.vtbl_ = &pchannelobvtbl_;
    impl->channelob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->client_ = client;
    impl->sd_ = sd;
    impl->muxer_ = muxer;

    aug_retain(mpool);
    return &impl->channelob_;

 fail2:
    aug_setfdeventmask(muxer, sd, 0);

 fail1:
    aug_destroytcpclient(client);
    return NULL;
}

AUGNET_API aug_channelob*
aug_createserver(aug_mpool* mpool, aug_sd sd, aug_muxer_t muxer)
{
    return NULL;
}

AUGNET_API aug_channelob*
aug_createplain(aug_mpool* mpool, aug_sd sd, aug_muxer_t muxer)
{
    struct pimpl_* impl = aug_malloc(mpool, sizeof(struct pimpl_));
    if (!impl)
        return NULL;

    impl->channelob_.vtbl_ = &pchannelobvtbl_;
    impl->channelob_.impl_ = NULL;
    impl->streamob_.vtbl_ = &pstreamobvtbl_;
    impl->streamob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->sd_ = sd;
    impl->muxer_ = muxer;

    aug_retain(mpool);
    return &impl->channelob_;
}
