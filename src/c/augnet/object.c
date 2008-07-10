/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/inet.h"   /* aug_setnodelay() */
#include "augnet/ssl.h"
#include "augnet/tcpconnect.h"

#include "augsys/base.h"   /* aug_nextid() */
#include "augsys/object.h" /* aug_safeestab() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include "augext/stream.h"

#include <assert.h>
#include <string.h>

struct cimpl_ {
    aug_chan chan_;
    int refs_;
    aug_mpool* mpool_;
    aug_muxer_t muxer_;
    unsigned id_;
    aug_sd sd_;
    char name_[AUG_MAXCHANNAMELEN + 1];
    unsigned short mask_;
    struct ssl_ctx_st* sslctx_;
    aug_tcpconnect_t conn_;
    int est_;
};

static aug_chan*
estabclient_(struct cimpl_* impl, aug_chandler* handler)
{
    aug_sd sd = impl->sd_;
    aug_chan* chan;
    aug_stream* stream;
    aug_bool ret;

    /* Ensure connection establishment happens only once. */

    impl->sd_ = AUG_BADSD;
    impl->est_ = 0;

    chan =
#if ENABLE_SSL
        impl->sslctx_ ?
        aug_createsslclient(impl->mpool_, impl->muxer_, impl->id_, sd,
                            impl->mask_, impl->sslctx_) :
#endif /* ENABLE_SSL */
        aug_createplain(impl->mpool_, impl->muxer_, impl->id_, sd,
                        impl->mask_);

    if (!chan) {
        aug_sclose(sd);
        return NULL;
    }

    /* Transfer event mask to established channel. */

    stream = aug_cast(chan, aug_streamid);

    /* Notification of connection establishment. */

    ret = aug_safeestab(&impl->chan_, handler, impl->id_, stream, impl->id_);
    aug_release(stream);

    if (!ret) {

        /* Rejected: socket will be closed on release. */

        aug_release(chan);
        return NULL;
    }

    return chan;
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
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_chanid)) {
        aug_retain(&impl->chan_);
        return &impl->chan_;
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
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
cchan_cast_(aug_chan* ob, const char* id)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    return ccast_(impl, id);
}

static void
cchan_retain_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    cretain_(impl);
}

static void
cchan_release_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    crelease_(impl);
}

static aug_result
cchan_close_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    aug_result result = cclose_(impl);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_chan*
cchan_process_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    if (impl->est_) {

        /* Descriptor will either be owned by new object or closed on
           error. */

        return estabclient_(impl, handler);
    }

    if ((AUG_FDEVENTCONN & aug_fdevents(impl->muxer_, impl->sd_))) {

        struct aug_endpoint ep;

        /* De-register existing descriptor from multiplexer, and attempt to
           establish connection. */

        if (aug_setfdeventmask(impl->muxer_, impl->sd_, 0) < 0
            || AUG_BADSD == (impl->sd_ = aug_tryconnect(impl->conn_, &ep,
                                                        &impl->est_)))
            return NULL;

        /* Update name based on new address. */

        aug_endpointntop(&ep, impl->name_, sizeof(impl->name_));

        if (impl->est_) {

            /* Descriptor will either be owned by new object or closed on
               error. */

            return estabclient_(impl, handler);
        }

        /* Not yet established: set mask to poll for connection
           establishment. */

        aug_setfdeventmask(impl->muxer_, impl->sd_, AUG_FDEVENTCONN);
    }

    cretain_(impl);
    return ob;
}

static aug_result
cchan_setmask_(aug_chan* ob, unsigned short mask)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    /* Mask will be set once the connection is established. */

    impl->mask_ = mask;
    return AUG_SUCCESS;
}

static int
cchan_getmask_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    return impl->mask_;
}

static unsigned
cchan_getid_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    return impl->id_;
}

static char*
cchan_getname_(aug_chan* ob, char* dst, unsigned size)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    aug_strlcpy(dst, impl->name_, size);
    return dst;
}

static const struct aug_chanvtbl cchanvtbl_ = {
    cchan_cast_,
    cchan_retain_,
    cchan_release_,
    cchan_close_,
    cchan_process_,
    cchan_setmask_,
    cchan_getmask_,
    cchan_getid_,
    cchan_getname_
};

struct simpl_ {
    aug_chan chan_;
    int refs_;
    aug_mpool* mpool_;
    aug_muxer_t muxer_;
    unsigned id_;
    aug_sd sd_;
    unsigned short mask_;
    struct ssl_ctx_st* sslctx_;
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
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_chanid)) {
        aug_retain(&impl->chan_);
        return &impl->chan_;
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
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
schan_cast_(aug_chan* ob, const char* id)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);
    return scast_(impl, id);
}

static void
schan_retain_(aug_chan* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);
    sretain_(impl);
}

static void
schan_release_(aug_chan* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);
    srelease_(impl);
}

static aug_result
schan_close_(aug_chan* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);
    aug_result result = sclose_(impl);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_chan*
schan_process_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);

    if ((AUG_FDEVENTRD & aug_fdevents(impl->muxer_, impl->sd_))) {

        aug_sd sd;
        struct aug_endpoint ep;
        aug_chan* chan;
        aug_stream* stream;
        unsigned id;
        aug_bool ret;

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
        chan =
#if ENABLE_SSL
            impl->sslctx_ ?
            aug_createsslserver(impl->mpool_, impl->muxer_, id, sd,
                                impl->mask_, impl->sslctx_) :
#endif /* ENABLE_SSL */
            aug_createplain(impl->mpool_, impl->muxer_, id, sd, impl->mask_);

        if (!chan) {
            aug_ctxwarn(aug_tlx, "aug_createplain() or aug_createsslserver()"
                        " failed: %s", aug_tlerr->desc_);
            aug_sclose(sd);
            goto done;
        }

        /* Transfer event mask to established channel. */

        aug_setchanmask(chan, impl->mask_);

        stream = aug_cast(chan, aug_streamid);

        /* Notification of connection establishment. */

        ret = aug_safeestab(ob, handler, id, stream, impl->id_);
        aug_release(stream);

        if (!ret) {

            /* Rejected: socket will be closed on release. */

            aug_release(chan);
            goto done;
        }

        /* Newly established connection. */

        *fork = AUG_TRUE;
        return chan;
    }

 done:
    sretain_(impl);
    return ob;
}

static aug_result
schan_setmask_(aug_chan* ob, unsigned short mask)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);

    /* Mask will be set for each subsequently accepted connection. */

    impl->mask_ = mask;
    return AUG_SUCCESS;
}

static int
schan_getmask_(aug_chan* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);
    return impl->mask_;
}

static unsigned
schan_getid_(aug_chan* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);
    return impl->id_;
}

static char*
schan_getname_(aug_chan* ob, char* dst, unsigned size)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);
    struct aug_endpoint ep;

    if (!aug_getsockname(impl->sd_, &ep) || !aug_endpointntop(&ep, dst, size))
        return NULL;

    return dst;
}

static const struct aug_chanvtbl schanvtbl_ = {
    schan_cast_,
    schan_retain_,
    schan_release_,
    schan_close_,
    schan_process_,
    schan_setmask_,
    schan_getmask_,
    schan_getid_,
    schan_getname_
};

struct pimpl_ {
    aug_chan chan_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_muxer_t muxer_;
    unsigned id_;
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
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
pchan_cast_(aug_chan* ob, const char* id)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    return pcast_(impl, id);
}

static void
pchan_retain_(aug_chan* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    pretain_(impl);
}

static void
pchan_release_(aug_chan* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    prelease_(impl);
}

static aug_result
pchan_close_(aug_chan* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    aug_result result = pclose_(impl);
    impl->sd_ = AUG_BADSD;
    return result;
}

static aug_chan*
pchan_process_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    int events = aug_fdevents(impl->muxer_, impl->sd_);
    if (events < 0)
        return NULL;

    if (events && !aug_safeready(ob, handler, impl->id_, &impl->stream_,
                                 events))
        return NULL;

    pretain_(impl);
    return ob;
}

static aug_result
pchan_setmask_(aug_chan* ob, unsigned short mask)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    return aug_setfdeventmask(impl->muxer_, impl->sd_, mask);
}

static int
pchan_getmask_(aug_chan* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    return aug_fdeventmask(impl->muxer_, impl->sd_);
}

static unsigned
pchan_getid_(aug_chan* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    return impl->id_;
}

static char*
pchan_getname_(aug_chan* ob, char* dst, unsigned size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    struct aug_endpoint ep;

    if (!aug_getpeername(impl->sd_, &ep) || !aug_endpointntop(&ep, dst, size))
        return NULL;

    return dst;
}

static const struct aug_chanvtbl pchanvtbl_ = {
    pchan_cast_,
    pchan_retain_,
    pchan_release_,
    pchan_close_,
    pchan_process_,
    pchan_setmask_,
    pchan_getmask_,
    pchan_getid_,
    pchan_getname_
};

static void*
pstream_cast_(aug_stream* ob, const char* id)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    return pcast_(impl, id);
}

static void
pstream_retain_(aug_stream* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    pretain_(impl);
}

static void
pstream_release_(aug_stream* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    prelease_(impl);
}

static aug_result
pstream_shutdown_(aug_stream* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    return aug_sshutdown(impl->sd_, SHUT_WR);
}

static ssize_t
pstream_read_(aug_stream* ob, void* buf, size_t size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    return aug_sread(impl->sd_, buf, size);
}

static ssize_t
pstream_readv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    return aug_sreadv(impl->sd_, iov, size);
}

static ssize_t
pstream_write_(aug_stream* ob, const void* buf, size_t size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    return aug_swrite(impl->sd_, buf, size);
}

static ssize_t
pstream_writev_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    return aug_swritev(impl->sd_, iov, size);
}

static const struct aug_streamvtbl pstreamvtbl_ = {
    pstream_cast_,
    pstream_retain_,
    pstream_release_,
    pstream_shutdown_,
    pstream_read_,
    pstream_readv_,
    pstream_write_,
    pstream_writev_
};

AUGNET_API aug_chan*
aug_createclient(aug_mpool* mpool, aug_muxer_t muxer, const char* host,
                 const char* serv, struct ssl_ctx_st* sslctx)
{
    aug_tcpconnect_t conn;
    aug_sd sd;
    char name[AUG_MAXCHANNAMELEN + 1];
    struct aug_endpoint ep;
    int est;
    struct cimpl_* impl;

    if (!(conn = aug_createtcpconnect(host, serv)))
        return NULL;

    if (AUG_BADSD == (sd = aug_tryconnect(conn, &ep, &est)))
        goto fail1;

    if (!(aug_endpointntop(&ep, name, sizeof(name))))
        goto fail2;

    if (est) {

        /* Now established.  Force multiplexer to return immediately so that
           establishment can be finalised in process() function. */

        aug_setnowait(muxer, 1);

    } else {

        /* Set mask to poll for connection establishment. */

        if (aug_setfdeventmask(muxer, sd, AUG_FDEVENTCONN) < 0)
            goto fail2;
    }

    if (!(impl = aug_allocmem(mpool, sizeof(struct cimpl_))))
        goto fail3;

    impl->chan_.vtbl_ = &cchanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->muxer_ = muxer;
    impl->id_ = aug_nextid();
    impl->sd_ = sd;
    strcpy(impl->name_, name);

    /* Default when established. */

    impl->mask_ = AUG_FDEVENTRD;
    impl->sslctx_ = sslctx;
    impl->conn_ = conn;
    impl->est_ = est;

    aug_retain(mpool);
    return &impl->chan_;

 fail3:

    if (est)
        aug_setnowait(muxer, -1);
    else
        aug_setfdeventmask(muxer, sd, 0);

 fail2:

    /* Owned by caller when established. */

    if (est)
        aug_sclose(sd);

 fail1:
    aug_destroytcpconnect(conn);
    return NULL;
}

AUGNET_API aug_chan*
aug_createserver(aug_mpool* mpool, aug_muxer_t muxer, aug_sd sd,
                 struct ssl_ctx_st* sslctx)
{
    struct simpl_* impl;

    if (aug_setfdeventmask(muxer, sd, AUG_FDEVENTRD) < 0)
        return NULL;

    if (!(impl = aug_allocmem(mpool, sizeof(struct simpl_)))) {
        aug_setfdeventmask(muxer, sd, 0);
        return NULL;
    }

    impl->chan_.vtbl_ = &schanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->muxer_ = muxer;
    impl->id_ = aug_nextid();
    impl->sd_ = sd;

    /* Default for new connections. */

    impl->mask_ = AUG_FDEVENTRD;
    impl->sslctx_ = sslctx;

    aug_retain(mpool);
    return &impl->chan_;
}

AUGNET_API aug_chan*
aug_createplain(aug_mpool* mpool, aug_muxer_t muxer, unsigned id, aug_sd sd,
                unsigned short mask)
{
    struct pimpl_* impl;

    if (aug_setfdeventmask(muxer, sd, mask) < 0)
        return NULL;

    if (!(impl = aug_allocmem(mpool, sizeof(struct pimpl_)))) {
        aug_setfdeventmask(muxer, sd, 0);
        return NULL;
    }

    impl->chan_.vtbl_ = &pchanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->stream_.vtbl_ = &pstreamvtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->muxer_ = muxer;
    impl->id_ = id;
    impl->sd_ = sd;

    aug_retain(mpool);
    return &impl->chan_;
}
