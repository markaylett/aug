/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/chan.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/inet.h"    /* aug_setnodelay() */
#include "augnet/ssl.h"
#include "augnet/tcpconnect.h"

#include "augsys/sticky.h"
#include "augsys/utility.h" /* aug_nextid() */
#include "augsys/uio.h"     /* struct aug_iovsum */

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
    char name_[AUG_MAXCHANNAMELEN + 1];
    unsigned id_;
    aug_muxer_t muxer_;
    aug_sd sd_;
    aug_bool wantwr_;
    struct ssl_ctx_st* sslctx_;
    aug_tcpconnect_t conn_;
    int est_;

    /* Fowarding pointer is used to allow re-entrant calls through old
       channel, before the new channel has fully replaced it. */

    aug_chan* fwd_;
};

static aug_chan*
estabclient_(struct cimpl_* impl, aug_chandler* handler)
{
    aug_sd sd = impl->sd_;

    /* Ensure connection establishment happens only once. */

    impl->sd_ = AUG_BADSD;
    impl->est_ = 0;

    AUG_CTXDEBUG3(aug_tlx, "client established: write=[%d]",
                  (int)impl->wantwr_);

    /* Create forward pointer. */

    impl->fwd_ =
#if WITH_SSL
        impl->sslctx_ ?
        aug_createsslclient(impl->mpool_, impl->id_, impl->muxer_, sd,
                            impl->wantwr_, handler, impl->sslctx_) :
#endif /* WITH_SSL */
        aug_createplain(impl->mpool_, impl->id_, impl->muxer_, sd,
                        impl->wantwr_);

    if (!impl->fwd_) {
        aug_errorchan(handler, &impl->chan_, aug_tlerr);
        aug_sclose(sd);
        return NULL;
    }

    /* Forward pointer has retained channel.  Retained channel owns socket
       descriptor. */

    /* Notify of connection establishment.  Parent-id is the same as
       channel-id for client connections. */

    if (!aug_estabchan(handler, impl->fwd_, impl->id_)) {

        /* Forward pointer still has retained channel.  The forward pointer
           represents the new state, even if rejected.  It will be released on
           destruction. */

        return NULL;
    }

    /* Always retain before return.  Two refs for forward pointer and
       return. */

    aug_retain(impl->fwd_);
    return impl->fwd_;
}

static aug_bool
error_(aug_chandler* handler, aug_chan* chan, aug_sd sd,
       unsigned short events)
{
    /* Exceptions may include non-error exceptions, such as high priority
       data. */

    if ((AUG_MDEVENTEX & events)) {

        /* Set socket-level error if one exists. */

        struct aug_errinfo errinfo;
        aug_setsockerrinfo(&errinfo, __FILE__, __LINE__, sd);

        if (errinfo.num_) {

            /* Error occurred. */

            aug_errorchan(handler, chan, &errinfo);
            return AUG_TRUE;
        }
    }

    return AUG_FALSE;
}

static aug_result
cclose_(struct cimpl_* impl)
{
    aug_result result = aug_setmdeventmask(impl->muxer_, impl->sd_, 0);

    /* Exceptional case: close if handshake is complete, but creation of
       forward pointer is still pending.  This means that conn_ has
       relinquished ownership of the socket descriptor, but ownership has not
       yet been transferred to the forward pointer. */

    if (impl->est_) {

        assert(!impl->fwd_);
        aug_sclose(impl->sd_);
    }

    return result;
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
        aug_destroytcpconnect(impl->conn_);
        if (AUG_BADSD != impl->sd_)
            cclose_(impl);
        aug_safeassign(impl->fwd_, NULL);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
cchan_cast_(aug_chan* ob, const char* id)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    if (impl->fwd_)
        return aug_cast(impl->fwd_, id);

    return ccast_(impl, id);
}

static void
cchan_retain_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    /* Not forwarded. */

    cretain_(impl);
}

static void
cchan_release_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    /* Not forwarded. */

    crelease_(impl);
}

static aug_result
cchan_close_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    aug_result result;

    if (impl->fwd_)
        return aug_closechan(impl->fwd_);

    result = cclose_(impl);
    impl->sd_ = AUG_BADSD;
    impl->est_ = 0;
    return result;
}

static aug_chan*
cchan_process_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);
    unsigned short events;

    if (impl->fwd_) {

        /* Added for completeness, although really not a valid case:
           aug_processchan() should not be called on old channel object. */

        return aug_processchan(impl->fwd_, handler, fork);
    }

    /* Channel closed. */

    if (AUG_BADSD == impl->sd_)
        return NULL;

    /* Was connection established on construction? */

    if (impl->est_) {

        /* Descriptor will either be owned by new object or closed on
           error. */

        return estabclient_(impl, handler);
    }

    events = aug_getmdevents(impl->muxer_, impl->sd_);

    /* Close socket on error. */

    if (error_(handler, &impl->chan_, impl->sd_, events))
        return NULL;

    if ((AUG_MDEVENTCONN & events)) {

        struct aug_endpoint ep;

        AUG_CTXDEBUG3(aug_tlx, "connection events: events=[%s]",
                      aug_eventlabel(events));

        /* De-register existing descriptor from multiplexer, and attempt to
           establish connection. */

        if (AUG_ISFAIL(aug_setmdeventmask(impl->muxer_, impl->sd_, 0))
            || AUG_BADSD == (impl->sd_ = aug_tryconnect(impl->conn_, &ep,
                                                        &impl->est_))) {
            aug_errorchan(handler, &impl->chan_, aug_tlerr);
            return NULL;
        }

        /* Update name based on new address. */

        aug_endpointntop(&ep, impl->name_, sizeof(impl->name_));

        if (impl->est_) {

            /* Descriptor will either be owned by new object or closed on
               error. */

            return estabclient_(impl, handler);
        }

        /* Not yet established: set mask to poll for connection
           establishment. */

        aug_setmdeventmask(impl->muxer_, impl->sd_, AUG_MDEVENTCONN);
    }

    cretain_(impl);
    return ob;
}

static aug_result
cchan_setwantwr_(aug_chan* ob, aug_bool wantwr)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    if (impl->fwd_)
        return aug_setchanwantwr(impl->fwd_, wantwr);

    /* Mask will be set once the connection is established. */

    AUG_CTXDEBUG3(aug_tlx, "set client wantwr: wr=[%d]", (int)wantwr);

    impl->wantwr_ = wantwr;
    return AUG_SUCCESS;
}

static unsigned
cchan_getid_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    if (impl->fwd_)
        return aug_getchanid(impl->fwd_);

    return impl->id_;
}

static char*
cchan_getname_(aug_chan* ob, char* dst, unsigned size)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    if (impl->fwd_)
        return aug_getchanname(impl->fwd_, dst, size);

    aug_strlcpy(dst, impl->name_, size);
    return dst;
}

static aug_bool
cchan_isready_(aug_chan* ob)
{
    struct cimpl_* impl = AUG_PODIMPL(struct cimpl_, chan_, ob);

    if (impl->fwd_)
        return aug_ischanready(impl->fwd_);

    /* True if closed. */

    if (AUG_BADSD == impl->sd_)
        return AUG_TRUE;

    /* The established flag may have been set on construction.  In which case,
       a process call is required to notify of establishment. */

    return impl->est_ ? AUG_TRUE : AUG_FALSE;
}

static const struct aug_chanvtbl cchanvtbl_ = {
    cchan_cast_,
    cchan_retain_,
    cchan_release_,
    cchan_close_,
    cchan_process_,
    cchan_setwantwr_,
    cchan_getid_,
    cchan_getname_,
    cchan_isready_
};

struct simpl_ {
    aug_chan chan_;
    int refs_;
    aug_mpool* mpool_;
    unsigned id_;
    aug_muxer_t muxer_;
    aug_sd sd_;
    aug_bool wantwr_;
    struct ssl_ctx_st* sslctx_;
};

static aug_result
sclose_(struct simpl_* impl)
{
    aug_setmdeventmask(impl->muxer_, impl->sd_, 0);
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
    unsigned short events;

    /* Channel closed. */

    if (AUG_BADSD == impl->sd_)
        return NULL;

    events = aug_getmdevents(impl->muxer_, impl->sd_);

    /* Close socket on error. */

    if (error_(handler, &impl->chan_, impl->sd_, events))
        return NULL;

    /* Assumption: server sockets do not have exceptional events. */

    if ((AUG_MDEVENTRD & events)) {

        aug_sd sd;
        struct aug_endpoint ep;
        aug_chan* chan;
        unsigned id;

        if (AUG_BADSD == (sd = aug_accept(impl->sd_, &ep))) {

            if (!aug_acceptagain(aug_tlerr)) {
                aug_errorchan(handler, &impl->chan_, aug_tlerr);
                return NULL;
            }

            aug_ctxwarn(aug_tlx, "aug_accept() failed: %s", aug_tlerr->desc_);
            goto done;
        }

        if (AUG_ISFAIL(aug_setnodelay(sd, AUG_TRUE))) {
            aug_ctxwarn(aug_tlx, "aug_setnodelay() failed: %s",
                        aug_tlerr->desc_);
            aug_sclose(sd);
            goto done;
        }

        if (AUG_ISFAIL(aug_ssetnonblock(sd, AUG_TRUE))) {
            aug_ctxwarn(aug_tlx, "aug_ssetnonblock() failed: %s",
                        aug_tlerr->desc_);
            aug_sclose(sd);
            goto done;
        }

        id = aug_nextid();
        chan =
#if WITH_SSL
            impl->sslctx_ ?
            aug_createsslserver(impl->mpool_, id, impl->muxer_, sd,
                                impl->wantwr_, handler, impl->sslctx_) :
#endif /* WITH_SSL */
            aug_createplain(impl->mpool_, id, impl->muxer_, sd,
                            impl->wantwr_);

        if (!chan) {
            aug_ctxwarn(aug_tlx, "aug_createplain() or aug_createsslserver()"
                        " failed: %s", aug_tlerr->desc_);
            aug_sclose(sd);
            goto done;
        }

        /* Notification of connection establishment. */

        if (!aug_estabchan(handler, chan, impl->id_)) {

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
schan_setwantwr_(aug_chan* ob, aug_bool wantwr)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);

    /* Mask will be set for each subsequently accepted connection. */

    AUG_CTXDEBUG3(aug_tlx, "set server wantwr: wr=[%d]", (int)wantwr);

    impl->wantwr_ = wantwr;
    return AUG_SUCCESS;
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

static aug_bool
schan_isready_(aug_chan* ob)
{
    struct simpl_* impl = AUG_PODIMPL(struct simpl_, chan_, ob);

    /* True if closed. */

    return AUG_BADSD == impl->sd_;
}

static const struct aug_chanvtbl schanvtbl_ = {
    schan_cast_,
    schan_retain_,
    schan_release_,
    schan_close_,
    schan_process_,
    schan_setwantwr_,
    schan_getid_,
    schan_getname_,
    schan_isready_
};

struct pimpl_ {
    aug_chan chan_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    unsigned id_;
    struct aug_sticky sticky_;
};

static aug_result
pclose_(struct pimpl_* impl)
{
    aug_sd sd = impl->sticky_.md_;

    /* Descriptor will be reset to AUG_BADMD. */

    aug_termsticky(&impl->sticky_);
    return aug_sclose(sd);
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
        if (AUG_BADMD != impl->sticky_.md_)
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
    return pclose_(impl);
}

static aug_chan*
pchan_process_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    unsigned short events;

    /* Channel closed. */

    if (AUG_BADMD == impl->sticky_.md_)
        return NULL;

    events = aug_getsticky(&impl->sticky_);

    /* Close socket on error. */

    if (error_(handler, &impl->chan_, impl->sticky_.md_, events))
        return NULL;

    if (events && !aug_readychan(handler, &impl->chan_, events))
        return NULL;

    pretain_(impl);
    return ob;
}

static aug_result
pchan_setwantwr_(aug_chan* ob, aug_bool wantwr)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);
    AUG_CTXDEBUG3(aug_tlx, "set plain wantwr: wr=[%d]", (int)wantwr);
    return aug_setsticky(&impl->sticky_, wantwr
                         ? AUG_MDEVENTALL : AUG_MDEVENTRDEX);
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

    if (!aug_getpeername(impl->sticky_.md_, &ep)
        || !aug_endpointntop(&ep, dst, size))
        return NULL;

    return dst;
}

static aug_bool
pchan_isready_(aug_chan* ob)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, chan_, ob);

    /* True if closed. */

    if (AUG_BADMD == impl->sticky_.md_)
        return AUG_TRUE;

    return aug_getsticky(&impl->sticky_) ? AUG_TRUE : AUG_FALSE;
}

static const struct aug_chanvtbl pchanvtbl_ = {
    pchan_cast_,
    pchan_retain_,
    pchan_release_,
    pchan_close_,
    pchan_process_,
    pchan_setwantwr_,
    pchan_getid_,
    pchan_getname_,
    pchan_isready_
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
    return aug_sshutdown(impl->sticky_.md_, SHUT_WR);
}

static aug_rsize
pstream_read_(aug_stream* ob, void* buf, size_t size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    aug_rsize rsize = aug_sread(impl->sticky_.md_, buf, size);
    if (AUG_ISBLOCK(rsize))
        aug_clearsticky(&impl->sticky_, AUG_MDEVENTRD);
    return rsize;
}

static aug_rsize
pstream_readv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    aug_rsize rsize = aug_sreadv(impl->sticky_.md_, iov, size);
    if (AUG_ISBLOCK(rsize))
        aug_clearsticky(&impl->sticky_, AUG_MDEVENTRD);
    return rsize;
}

static aug_rsize
pstream_write_(aug_stream* ob, const void* buf, size_t size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    aug_rsize rsize = aug_swrite(impl->sticky_.md_, buf, size);
    if (AUG_ISBLOCK(rsize))
        aug_clearsticky(&impl->sticky_, AUG_MDEVENTWR);
    return rsize;
}

static aug_rsize
pstream_writev_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct pimpl_* impl = AUG_PODIMPL(struct pimpl_, stream_, ob);
    aug_rsize rsize = aug_swritev(impl->sticky_.md_, iov, size);
    if (AUG_ISBLOCK(rsize))
        aug_clearsticky(&impl->sticky_, AUG_MDEVENTWR);
    return rsize;
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
aug_createclient(aug_mpool* mpool, const char* host, const char* serv,
                 aug_muxer_t muxer, struct ssl_ctx_st* sslctx)
{
    aug_tcpconnect_t conn;
    aug_sd sd;
    char name[AUG_MAXCHANNAMELEN + 1];
    struct aug_endpoint ep;
    int est;
    struct cimpl_* impl;

    if (!(conn = aug_createtcpconnect(mpool, host, serv)))
        return NULL;

    if (AUG_BADSD == (sd = aug_tryconnect(conn, &ep, &est)))
        goto fail1;

    if (!(aug_endpointntop(&ep, name, sizeof(name))))
        goto fail2;

    if (!est) {

        /* Set mask to poll for connection establishment. */

        if (AUG_ISFAIL(aug_setmdeventmask(muxer, sd, AUG_MDEVENTCONN)))
            goto fail2;
    }

    if (!(impl = aug_allocmem(mpool, sizeof(struct cimpl_))))
        goto fail3;

    impl->chan_.vtbl_ = &cchanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    strcpy(impl->name_, name);
    impl->id_ = aug_nextid();
    impl->muxer_ = muxer;
    impl->sd_ = sd;

    /* Default when established. */

    impl->wantwr_ = AUG_FALSE;
    impl->sslctx_ = sslctx;
    impl->conn_ = conn;
    impl->est_ = est;
    impl->fwd_ = NULL;

    aug_retain(mpool);
    return &impl->chan_;

 fail3:

    if (!est)
        aug_setmdeventmask(muxer, sd, 0);

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

    /* A readable event will be delivered when a new connection is
       attempted. */

    if (AUG_ISFAIL(aug_setmdeventmask(muxer, sd, AUG_MDEVENTRDEX)))
        return NULL;

    if (!(impl = aug_allocmem(mpool, sizeof(struct simpl_)))) {
        aug_setmdeventmask(muxer, sd, 0);
        return NULL;
    }

    impl->chan_.vtbl_ = &schanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->id_ = aug_nextid();
    impl->muxer_ = muxer;
    impl->sd_ = sd;

    /* Default mask for new connections. */

    impl->wantwr_ = AUG_FALSE;
    impl->sslctx_ = sslctx;

    aug_retain(mpool);
    return &impl->chan_;
}

AUGNET_API aug_chan*
aug_createplain(aug_mpool* mpool, unsigned id, aug_muxer_t muxer, aug_sd sd,
                aug_bool wantwr)
{
    struct pimpl_* impl = aug_allocmem(mpool, sizeof(struct pimpl_));
    if (!impl)
        return NULL;

    impl->chan_.vtbl_ = &pchanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->stream_.vtbl_ = &pstreamvtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->id_ = id;

    /* Sticky event flags are used for edge-triggered interfaces. */

    if (AUG_ISFAIL(aug_initsticky(&impl->sticky_, muxer, sd, wantwr
                                  ? AUG_MDEVENTALL : AUG_MDEVENTRDEX))) {
        aug_freemem(mpool, impl);
        return NULL;
    }

    aug_retain(mpool);
    return &impl->chan_;
}
