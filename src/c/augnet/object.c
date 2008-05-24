/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/inet.h" /* aug_setnodelay() */
#include "augnet/ssl.h"
#include "augnet/tcpconnect.h"

#include "augsys/base.h" /* aug_nextid() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augob/streamob.h"

#include <assert.h>
#include <string.h>

static aug_bool
safecb_(aug_channelob* channel, aug_channelcb_t cb, aug_object* cbob,
        unsigned id, aug_streamob* ob, unsigned short events)
{
    aug_bool ret;

    /* The callback is not required to set errinfo when returning false.  The
       errinfo record must therefore be cleared before the callback is made,
       to avoid any confusion with previous errors. */

    aug_clearerrinfo(aug_tlerr);

    /* Lock here to prevent release during callback. */

    aug_retain(channel);
    ret = cb(cbob, id, ob, events);
    aug_release(channel);

    return ret;
}

struct cimpl_ {
    aug_channelob channelob_;
    int refs_;
    aug_mpool* mpool_;
    unsigned id_;
    aug_muxer_t muxer_;
    aug_sd sd_;
    unsigned short mask_;
    struct ssl_st* ssl_;
    aug_tcpconnect_t conn_;
    int est_;
};

static aug_channelob*
establish_(struct cimpl_* impl, aug_channelcb_t cb, aug_object* cbob)
{
    aug_sd sd = impl->sd_;
    aug_channelob* channelob;
    aug_streamob* streamob;

    /* Ensure connection establishment happens only once. */

    impl->sd_ = AUG_BADSD;
    impl->est_ = 0;

    channelob =
#if ENABLE_SSL
        impl->ssl_ ? aug_createsslclient(impl->mpool_, impl->id_,
                                         impl->muxer_, sd, impl->mask_,
                                         impl->ssl_) :
#endif /* ENABLE_SSL */
        aug_createplain(impl->mpool_, impl->id_, impl->muxer_, sd,
                        impl->mask_);

    if (!channelob) {
        aug_sclose(sd);
        return NULL;
    }

    /* Transfer event mask to established channel. */

    streamob = aug_cast(channelob, aug_streamobid);

    /* Zero events indicates new connection. */

    if (!safecb_(&impl->channelob_, cb, cbob, impl->id_, streamob, 0)) {

        /* Rejected: socket will be closed on release. */

        aug_release(streamob);
        aug_release(channelob);
        return NULL;
    }

    /* Transition to established state. */

    aug_release(streamob);
    return channelob;
}

static aug_result
cclose_(struct cimpl_* impl)
{
    /* Nothing to close because handshake is not yet complete. */

    return aug_setfdeventmask(impl->muxer_, impl->sd_, 0);
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
        aug_destroytcpconnect(impl->conn_);
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
cchannelob_process_(aug_channelob* ob, aug_bool* fork, aug_channelcb_t cb,
                    aug_object* cbob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);

    if (impl->est_) {

        /* Descriptor will either be owned by new object or closed on
           error. */

        return establish_(impl, cb, cbob);
    }

    if ((AUG_FDEVENTRD & aug_fdevents(impl->muxer_, impl->sd_))) {

        struct aug_endpoint ep;

        /* De-register existing descriptor from multiplexer, and attempt to
           establish connection. */

        if (aug_setfdeventmask(impl->muxer_, impl->sd_, 0) < 0
            || AUG_BADSD == (impl->sd_ = aug_tryconnect(impl->conn_, &ep,
                                                        &impl->est_)))
            return NULL;

        if (impl->est_) {

            /* Descriptor will either be owned by new object or closed on
               error. */

            return establish_(impl, cb, cbob);
        }

        /* Not yet established: set mask to poll for connection
           establishment. */

        aug_setfdeventmask(impl->muxer_, impl->sd_, AUG_FDEVENTCONN);
    }

    cretain_(impl);
    return ob;
}

static aug_result
cchannelob_seteventmask_(aug_channelob* ob, unsigned short mask)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);

    /* Mask will be set once the connection is established. */

    impl->mask_ = mask;
    return AUG_SUCCESS;
}

static unsigned
cchannelob_getid_(aug_channelob* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    return impl->id_;
}

static int
cchannelob_eventmask_(aug_channelob* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, channelob_, ob);
    return impl->mask_;
}

static const struct aug_channelobvtbl cchannelobvtbl_ = {
    cchannelob_cast_,
    cchannelob_retain_,
    cchannelob_release_,
    cchannelob_close_,
    cchannelob_process_,
    cchannelob_seteventmask_,
    cchannelob_getid_,
    cchannelob_eventmask_
};

struct simpl_ {
    aug_channelob channelob_;
    int refs_;
    aug_mpool* mpool_;
    unsigned id_;
    aug_muxer_t muxer_;
    aug_sd sd_;
    unsigned short mask_;
    struct ssl_st* ssl_;
};

static aug_result
sclose_(struct simpl_* impl)
{
    aug_setfdeventmask(impl->muxer_, impl->sd_, 0);
    return aug_sclose(impl->sd_);
}

static void*
scast_(struct simpl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_channelobid)) {
        aug_retain(&impl->channelob_);
        return &impl->channelob_;
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
            sclose_(impl);
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static void*
schannelob_cast_(aug_channelob* ob, const char* id)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);
    return scast_(impl, id);
}

static void
schannelob_retain_(aug_channelob* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);
    sretain_(impl);
}

static void
schannelob_release_(aug_channelob* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);
    srelease_(impl);
}

static aug_result
schannelob_close_(aug_channelob* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);
    aug_result result = sclose_(impl);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_channelob*
schannelob_process_(aug_channelob* ob, aug_bool* fork, aug_channelcb_t cb,
                    aug_object* cbob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);

    if ((AUG_FDEVENTRD & aug_fdevents(impl->muxer_, impl->sd_))) {

        aug_sd sd;
        struct aug_endpoint ep;
        aug_channelob* channelob;
        aug_streamob* streamob;
        unsigned id;

        if (AUG_BADSD == (sd = aug_accept(impl->sd_, &ep))) {

            if (!aug_acceptlost())
                return NULL; /* Error. */

            aug_ctxwarn(aug_tlx, "aug_accept() failed: %s", aug_tlerr->desc_);
            goto done;
        }

        if (-1 == aug_setnodelay(sd, AUG_TRUE)) {
            aug_ctxwarn(aug_tlx, "aug_setnodelay() failed: %s",
                        aug_tlerr->desc_);
            aug_sclose(sd);
            goto done;
        }

        if (-1 == aug_ssetnonblock(sd, AUG_TRUE)) {
            aug_ctxwarn(aug_tlx, "aug_ssetnonblock() failed: %s",
                        aug_tlerr->desc_);
            aug_sclose(sd);
            goto done;
        }

        id = aug_nextid();
        channelob =
#if ENABLE_SSL
            impl->ssl_ ? aug_createsslserver(impl->mpool_, id, impl->muxer_,
                                             sd, impl->mask_, impl->ssl_) :
#endif /* ENABLE_SSL */
            aug_createplain(impl->mpool_, id, impl->muxer_, sd, impl->mask_);

        if (!channelob) {
            aug_ctxwarn(aug_tlx, "aug_createplain() or aug_createsslserver()"
                        " failed: %s", aug_tlerr->desc_);
            aug_sclose(sd);
            goto done;
        }

        /* Transfer event mask to established channel. */

        aug_seteventmask(channelob, impl->mask_);

        streamob = aug_cast(ob, aug_streamobid);

        /* Zero events indicates new connection. */

        if (!safecb_(ob, cb, cbob, id, streamob, 0)) {

            /* Rejected: socket will be closed on release. */

            aug_release(streamob);
            aug_release(channelob);
            goto done;
        }

        /* Newly established connection. */

        *fork = AUG_TRUE;
        aug_release(streamob);
        return channelob;
    }

 done:
    sretain_(impl);
    return ob;
}

static aug_result
schannelob_seteventmask_(aug_channelob* ob, unsigned short mask)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);

    /* Mask will be set for each subsequently accepted connection. */

    impl->mask_ = mask;
    return AUG_SUCCESS;
}

static unsigned
schannelob_getid_(aug_channelob* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);
    return impl->id_;
}

static int
schannelob_eventmask_(aug_channelob* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, channelob_, ob);
    return impl->mask_;
}

static const struct aug_channelobvtbl schannelobvtbl_ = {
    schannelob_cast_,
    schannelob_retain_,
    schannelob_release_,
    schannelob_close_,
    schannelob_process_,
    schannelob_seteventmask_,
    schannelob_getid_,
    schannelob_eventmask_
};

struct pimpl_ {
    aug_channelob channelob_;
    aug_streamob streamob_;
    int refs_;
    aug_mpool* mpool_;
    unsigned id_;
    aug_muxer_t muxer_;
    aug_sd sd_;
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
pchannelob_process_(aug_channelob* ob, aug_bool* fork, aug_channelcb_t cb,
                    aug_object* cbob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    int events = aug_fdevents(impl->muxer_, impl->sd_);
    if (events < 0)
        return NULL;

    if (events && !safecb_(ob, cb, cbob, impl->id_, &impl->streamob_, events))
        return NULL;

    pretain_(impl);
    return ob;
}

static aug_result
pchannelob_seteventmask_(aug_channelob* ob, unsigned short mask)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return aug_setfdeventmask(impl->muxer_, impl->sd_, mask);
}

static unsigned
pchannelob_getid_(aug_channelob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return impl->id_;
}

static int
pchannelob_eventmask_(aug_channelob* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, channelob_, ob);
    return aug_fdeventmask(impl->muxer_, impl->sd_);
}

static const struct aug_channelobvtbl pchannelobvtbl_ = {
    pchannelob_cast_,
    pchannelob_retain_,
    pchannelob_release_,
    pchannelob_close_,
    pchannelob_process_,
    pchannelob_seteventmask_,
    pchannelob_getid_,
    pchannelob_eventmask_
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
                 aug_muxer_t muxer, struct ssl_st* ssl)
{
    aug_tcpconnect_t conn;
    aug_sd sd;
    struct aug_endpoint ep;
    int est;
    struct cimpl_* impl;

    if (!(conn = aug_createtcpconnect(host, serv)))
        return NULL;

    if (AUG_BADSD == (sd = aug_tryconnect(conn, &ep, &est)))
        goto fail1;

    if (est) {

        /* Now established.  Force multiplexer to return immediately so that
           establishment can be finalised in process() function. */

        aug_setnowait(impl->muxer_, 1);

    } else {

        /* Set mask to poll for connection establishment. */

        if (aug_setfdeventmask(muxer, sd, AUG_FDEVENTCONN) < 0)
            goto fail1;
    }

    if (!(impl = aug_malloc(mpool, sizeof(struct cimpl_))))
        goto fail2;

    impl->channelob_.vtbl_ = &cchannelobvtbl_;
    impl->channelob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->id_ = aug_nextid();
    impl->muxer_ = muxer;
    impl->sd_ = sd;

    /* Default when established. */

    impl->mask_ = AUG_FDEVENTRD;
    impl->ssl_ = ssl;
    impl->conn_ = conn;
    impl->est_ = est;

    aug_retain(mpool);
    return &impl->channelob_;

 fail2:
    aug_setfdeventmask(muxer, sd, 0);

 fail1:
    aug_destroytcpconnect(conn);
    return NULL;
}

AUGNET_API aug_channelob*
aug_createserver(aug_mpool* mpool, aug_muxer_t muxer, aug_sd sd,
                 struct ssl_st* ssl)
{
    struct simpl_* impl = aug_malloc(mpool, sizeof(struct simpl_));
    if (!impl)
        return NULL;

    impl->channelob_.vtbl_ = &schannelobvtbl_;
    impl->channelob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->id_ = aug_nextid();
    impl->muxer_ = muxer;
    impl->sd_ = sd;

    /* Default for new connections. */

    impl->mask_ = AUG_FDEVENTRD;
    impl->ssl_ = ssl;

    aug_retain(mpool);
    return &impl->channelob_;
}

AUGNET_API aug_channelob*
aug_createplain(aug_mpool* mpool, unsigned id, aug_muxer_t muxer, aug_sd sd,
                unsigned short mask)
{
    struct pimpl_* impl;

    if (aug_setfdeventmask(muxer, sd, mask) < 0)
        return NULL;

    if (!(impl = aug_malloc(mpool, sizeof(struct pimpl_)))) {
        aug_setfdeventmask(muxer, sd, 0);
        return NULL;
    }

    impl->channelob_.vtbl_ = &pchannelobvtbl_;
    impl->channelob_.impl_ = NULL;
    impl->streamob_.vtbl_ = &pstreamobvtbl_;
    impl->streamob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->id_ = id;
    impl->muxer_ = muxer;
    impl->sd_ = sd;

    aug_retain(mpool);
    return &impl->channelob_;
}
