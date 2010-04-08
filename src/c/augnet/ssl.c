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
#define AUGNET_BUILD
#include "augnet/ssl.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if WITH_SSL

# include "augsys/socket.h" /* aug_shutdown() */
# include "augsys/sticky.h"
# include "augsys/uio.h"

# include "augctx/base.h"
# include "augctx/errinfo.h"
# include "augctx/errno.h"

# include "augext/log.h"
# include "augext/stream.h"

# include <openssl/err.h>
# include <openssl/ssl.h>

# include <assert.h>
# include <string.h>        /* memcpy() */

/* Buffer-size should be reduced during testing. */

struct buf_ {
    char buf_[AUG_BUFSIZE];
    char* rd_, * wr_;
};

/* States are composed of sub-states. */

#define NEUTRAL_    0x01
#define READ_       0x02
#define WRITE_      0x04
#define PENDRD_     0x08
#define WANTRD_     0x10
#define WANTWR_     0x20

/* End of data. */

#define ENDOF_      0x40
#define ERROR_      0x80

/* Composite states. */

/* Bytes stored in SSL buffer awaiting SSL_read(). */

#define RDPENDRD_  (READ_  | PENDRD_)
#define RDWANTRD_  (READ_  | WANTRD_)
#define RDWANTWR_  (READ_  | WANTWR_)
#define WRWANTRD_  (WRITE_ | WANTRD_)
#define WRWANTWR_  (WRITE_ | WANTWR_)

/* WRWANTWR_ is also used to initiate the handshake. */

#define HANDSHAKE_ (WRITE_ | WANTWR_)

struct impl_ {
    aug_chan chan_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    struct aug_sticky sticky_;

    /* User wants write. */

    aug_bool wantwr_;
    struct aug_ssldata data_;
    SSL* ssl_;
    struct buf_ inbuf_, outbuf_;
    unsigned state_, except_;
    struct aug_errinfo errinfo_;
    aug_bool shutdown_;
};

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

static int
bufempty_(struct buf_* x)
{
    /* Return true if the buffer is empty. */

    return x->rd_ == x->wr_;
}

static aug_bool
buffull_(struct buf_* x)
{
    /* Return true if the buffer is full. */

    const char* end = x->buf_ + sizeof(x->buf_);
    return x->wr_ == end;
}

static void
clearbuf_(struct buf_* x)
{
    /* Reset pointers to start of buffer. */

    x->rd_ = x->wr_ = x->buf_;
}

static size_t
readbuf_(struct buf_* x, void* buf, size_t size)
{
    size_t ret = AUG_MIN((size_t)(x->wr_ - x->rd_), size);
    if (ret) {

        /* Copy bytes from buffer at read pointer. */

        memcpy(buf, x->rd_, ret);
        x->rd_ += ret;

        /* Clear if read pointer caught write. */

        if (bufempty_(x))
            clearbuf_(x);
    }
    return ret;
}

static size_t
writebuf_(struct buf_* x, const void* buf, size_t size)
{
    const char* end = x->buf_ + sizeof(x->buf_);
    size_t ret = AUG_MIN((size_t)(end - x->wr_), size);
    if (ret) {

        /* Copy bytes to buffer at write pointer. */

        memcpy(x->wr_, buf, ret);
        x->wr_ += ret;
    }
    return ret;
}

static size_t
readbufv_(struct buf_* x, const struct iovec* iov, int size)
{
    size_t n, ret = 0;

    /* For each entry in the vector. */

    for (; size; --size, ++iov) {

        n = readbuf_(x, iov->iov_base, iov->iov_len);
        ret += n;

        /* Not enough to satisfy entry. */

        if (n < (size_t)iov->iov_len)
            break;
    }
    return ret;
}

static size_t
writebufv_(struct buf_* x, const struct iovec* iov, int size)
{
    size_t n, ret = 0;

    /* For each entry in the vector. */

    for (; size; --size, ++iov) {

        n = writebuf_(x, iov->iov_base, iov->iov_len);
        ret += n;

        /* Not enough to satisfy entry. */

        if (n < (size_t)iov->iov_len)
            break;
    }
    return ret;
}

static int
getmask_(struct impl_* impl)
{
    /* Add write bit if either the SSL layer wants write, or there is buffered
       data to be written. */

    return (impl->state_ & WANTWR_)
        || !bufempty_(&impl->outbuf_)
        ? AUG_MDEVENTALL : AUG_MDEVENTRDEX;
}

static aug_bool
isexcept_(struct impl_* impl, unsigned short events)
{
    /* Exceptional event or state. */

    return (impl->state_ & (ENDOF_ | ERROR_))
        || (events & AUG_MDEVENTEX)
        ? AUG_TRUE : AUG_FALSE;
}

static aug_bool
isread_(struct impl_* impl, unsigned short events)
{
    /* Never read if blocked on write. */

    if (impl->state_ & WRITE_)
        return AUG_FALSE;

    /* Only consume a write event if read has explicitly asked for it. */

    return RDPENDRD_ == impl->state_
        || (events & AUG_MDEVENTRD)
        || ((events & AUG_MDEVENTWR) && RDWANTWR_ == impl->state_)
        ? AUG_TRUE : AUG_FALSE;
}

static aug_bool
iswrite_(struct impl_* impl, unsigned short events)
{
    /* Never write if blocked on read. */

    if (impl->state_ & READ_)
        return AUG_FALSE;

    /* Only consume a read event if write has explicitly asked for it. */

    return (events & AUG_MDEVENTWR)
        || ((events & AUG_MDEVENTRD) && WRWANTRD_ == impl->state_)
        ? AUG_TRUE : AUG_FALSE;
}

static aug_result
shutwr_(struct impl_* impl, struct aug_errinfo* errinfo)
{
    int ret;

    AUG_CTXDEBUG3(aug_tlx, "SSL: shutdown: id=[%d]", (int)impl->data_.id_);
    ret = SSL_shutdown(impl->ssl_);
    aug_sshutdown(impl->sticky_.md_, SHUT_WR);

    if (ret < 0) {
        aug_setsslerrinfo(errinfo, __FILE__, __LINE__, ERR_get_error());
        return -1;
    }
    return 0;
}

static ssize_t
sslread_(SSL* ssl, struct buf_* x)
{
    /* Read bytes into buffer at write pointer. */

    const char* end = x->buf_ + sizeof(x->buf_);
    ssize_t n = (ssize_t)(end - x->wr_);

    AUG_CTXDEBUG3(aug_tlx, "SSL: ssl read to input buffer");
    n = SSL_read(ssl, x->wr_, n);
    if (0 < n)
        x->wr_ += n;
    return n;
}

static ssize_t
sslwriteshake_(SSL* ssl, struct buf_* x)
{
    /* Write bytes from buffer at read pointer. */

    ssize_t n = (ssize_t)(x->wr_ - x->rd_);
    if (n) {
        AUG_CTXDEBUG3(aug_tlx, "SSL: ssl write from output buffer");
        n = SSL_write(ssl, x->rd_, n);
        if (0 < n) {
            x->rd_ += n;

            /* Clear if read pointer caught write. */

            if (bufempty_(x))
                clearbuf_(x);
        }
    } else {

        /* Output buffer is empty, try handshake. */

        AUG_CTXDEBUG3(aug_tlx, "SSL: handshake without write");
        n = SSL_do_handshake(ssl);
    }
    return n;
}

static unsigned short
readyevents_(struct impl_* impl)
{
    unsigned short events = 0;

    /* Channel is readable if there is either a state that needs
       communicating, or data in the input buffer.  Note: end-of-data and
       errors are communicated to the user via aug_read(). */

    if ((impl->state_ & (ENDOF_ | ERROR_)) || !bufempty_(&impl->inbuf_))
        events |= AUG_MDEVENTRD;

    if (impl->wantwr_ && !buffull_(&impl->outbuf_))
        events |= AUG_MDEVENTWR;

    return events;
}

static void
setmask_(struct impl_* impl)
{
    unsigned short mask = getmask_(impl);

    AUG_CTXDEBUG3(aug_tlx, "SSL: set mask: mask=[%s]", aug_eventlabel(mask));
    aug_setsticky(&impl->sticky_, mask);
}

static void
readwrite_(struct impl_* impl, unsigned short events)
{
    ssize_t ret;
    int err;

    if (isread_(impl, events) && !buffull_(&impl->inbuf_)) {

        /* Perform read operation. */

        ret = sslread_(impl->ssl_, &impl->inbuf_);
        err = SSL_get_error(impl->ssl_, ret);
        switch (err) {
        case SSL_ERROR_NONE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: %ld bytes read to input buffer",
                          (long)ret);
            if ((ret = SSL_pending(impl->ssl_))) {
                AUG_CTXDEBUG3(aug_tlx,
                              "SSL: %d bytes pending for immediate read",
                              ret);

                /* Pending data for immediate read. */

                impl->state_ = RDPENDRD_;
                return;
            }

            /* Proceed with write when state is neutral. */

            impl->state_ = NEUTRAL_;
            break;

        case SSL_ERROR_WANT_READ:
            AUG_CTXDEBUG3(aug_tlx, "SSL: read wants read");
            aug_clearsticky(&impl->sticky_, AUG_MDEVENTRD);
            impl->state_ = RDWANTRD_;
            return;
        case SSL_ERROR_WANT_WRITE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: read wants write");
            aug_clearsticky(&impl->sticky_, AUG_MDEVENTWR);
            impl->state_ = RDWANTWR_;
            return;
        case SSL_ERROR_SYSCALL:
            AUG_CTXDEBUG3(aug_tlx, "SSL: read syscall error", err);

            /* Save error locally. */

#if !defined(_WIN32)
            aug_setposixerrinfo(&impl->errinfo_, __FILE__, __LINE__, errno);
#else /* _WIN32 */
            aug_setwin32errinfo(&impl->errinfo_, __FILE__, __LINE__,
                                WSAGetLastError());
#endif /* _WIN32 */

            /* If system call error could not be obtained. */

            if (0 == impl->errinfo_.num_) {
                aug_seterrinfo(&impl->errinfo_, __FILE__, __LINE__, "aug",
                               AUG_EIO, "ssl read syscall error");
            }
            impl->state_ = ERROR_;
            return;
        case SSL_ERROR_ZERO_RETURN:
            AUG_CTXDEBUG3(aug_tlx, "SSL: end of data");
            if (!impl->shutdown_) {
                AUG_CTXDEBUG3(aug_tlx, "SSL: shutting-down", ret);
                SSL_shutdown(impl->ssl_);
            }
            impl->state_ = ENDOF_;
            return;
        default:

            AUG_CTXDEBUG3(aug_tlx, "SSL: error=[%d]", err);

            /* Save error locally. */

            aug_setsslerrinfo(&impl->errinfo_, __FILE__, __LINE__,
                              ERR_get_error());
            impl->state_ = ERROR_;
            return;
        }
    }

    if (iswrite_(impl, events)) {

        /* Perform write operation.  If output buffer is empty,
           sslwriteshake_() will perform SSL_do_handshake(). */

        ret = sslwriteshake_(impl->ssl_, &impl->outbuf_);
        err = SSL_get_error(impl->ssl_, ret);
        switch (err) {
        case SSL_ERROR_NONE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: %d bytes written from output buffer",
                          ret);

            /* If shutdown is pending and output buffer is now empty, do
               shutdown. */

            if (impl->shutdown_ && bufempty_(&impl->outbuf_)
                && shutwr_(impl, &impl->errinfo_) < 0) {

                /* Save error locally. */

                impl->state_ = ERROR_;
            }

            impl->state_ = NEUTRAL_;
            break;
        case SSL_ERROR_WANT_READ:
            AUG_CTXDEBUG3(aug_tlx, "SSL: write wants read");
            aug_clearsticky(&impl->sticky_, AUG_MDEVENTRD);
            impl->state_ = WRWANTRD_;
            break;
        case SSL_ERROR_WANT_WRITE:
            AUG_CTXDEBUG3(aug_tlx, "SSL: write wants write");
            aug_clearsticky(&impl->sticky_, AUG_MDEVENTWR);
            impl->state_ = WRWANTWR_;
            break;
        case SSL_ERROR_SYSCALL:
            AUG_CTXDEBUG3(aug_tlx, "SSL: write syscall error", err);

            /* Save error locally. */

            impl->except_ =
#if !defined(_WIN32)
                aug_setposixerrinfo(&impl->errinfo_, __FILE__, __LINE__,
                                    errno);
#else /* _WIN32 */
                aug_setwin32errinfo(&impl->errinfo_, __FILE__, __LINE__,
                                    WSAGetLastError());
#endif /* _WIN32 */

            /* If system call error could not be obtained. */

            if (0 == impl->errinfo_.num_) {
                aug_seterrinfo(&impl->errinfo_, __FILE__, __LINE__, "aug",
                               AUG_EIO, "ssl write syscall error");
            }
            impl->state_ = ERROR_;
            break;
        default:

            AUG_CTXDEBUG3(aug_tlx, "SSL: error=[%d]", err);

            /* Save error locally. */

            aug_setsslerrinfo(&impl->errinfo_, __FILE__, __LINE__,
                              ERR_get_error());
            impl->state_ = ERROR_;
            break;
        }
    }
}

static aug_result
close_(struct impl_* impl)
{
    aug_sd sd = impl->sticky_.md_;

    AUG_CTXDEBUG3(aug_tlx, "SSL: clearing io-event mask: id=[%d]",
                  (int)impl->data_.id_);

    /* Descriptor will be reset to AUG_BADMD. */

    aug_termsticky(&impl->sticky_);
    return aug_sclose(sd);
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
        if (AUG_BADMD != impl->sticky_.md_)
            close_(impl);
        SSL_free(impl->ssl_);
        aug_release(impl->data_.handler_);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
ccast_(aug_chan* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return cast_(impl, id);
}

static void
cretain_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    retain_(impl);
}

static void
crelease_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    release_(impl);
}

static aug_result
cclose_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    AUG_CTXDEBUG3(aug_tlx, "SSL: closing file: id=[%d]",
                  (int)impl->data_.id_);
    return close_(impl);
}

static aug_chan*
cprocess_BI_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    unsigned short events;

    /* Channel closed. */

    if (AUG_BADMD == impl->sticky_.md_)
        return NULL;

    events = aug_getsticky(&impl->sticky_);

    /* Close socket on error. */

    if (error_(handler, &impl->chan_, impl->sticky_.md_, events))
        return NULL;

    /* TODO: handle remaining exceptional events. */

    /* If neutral and read event is set, then test readability before entering
       SSL_read() operation. */

    if (NEUTRAL_ == impl->state_ && (events & AUG_MDEVENTRD)) {

        /* SSL_read() returns SSL_ERROR_WANT_READ if the operation would have
           blocked.  This results in a state of RDWANTRD_, which means that
           the SSL_read() will be resumed once the socket becomes readable
           again.

           If the initial SSL_read() had been initiated by the AUG_MDEVENTRD
           sticky bit, when the socket was not actually readable, then this
           could inadvertently starve any writes, as they will not be serviced
           until the SSL_read() operation is complete.

           To work around this, MSG_PEEK is used to ensure that the socket is
           readable before issuing an SSL_read(). */

        char ch;

        /* EXCEPT: cprocess_BI_ -> aug_recv_BI; */

        if (aug_recv_BI(impl->sticky_.md_, &ch, 1, MSG_PEEK) < 0
            && AUG_EXBLOCK == aug_getexcept(aug_tlx)) {

            /* If the peek operation would have blocked, then clear the sticky
               bit. */

            AUG_CTXDEBUG3(aug_tlx, "SSL: clearing blocked read");
            aug_clearsticky(&impl->sticky_, AUG_MDEVENTRD);
            events &= ~AUG_MDEVENTRD;
        }
    }

    if (events)
        readwrite_(impl, events);
    else
        AUG_CTXDEBUG3(aug_tlx, "SSL: readwrite_() skipped: state=[%u],"
                      " sticky=[%s]", (unsigned)impl->state_,
                      aug_eventlabel(events));

    events = readyevents_(impl);
    AUG_CTXDEBUG3(aug_tlx, "SSL: readyevents_(): ready=[%s]",
                  aug_eventlabel(events));

    if (events) {

        /* Returns false if channel is to be removed. */

        if (!aug_readychan(handler, &impl->chan_, events)) {

            /* No need to update events if file is being removed - indicated
               by false return. */

            return NULL;
        }
    }

    /* Update muxer's mask. */

    setmask_(impl);
    retain_(impl);
    return ob;
}

static aug_result
csetwantwr_(aug_chan* ob, aug_bool wantwr)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);

    AUG_CTXDEBUG3(aug_tlx, "SSL: set wantwr: id=[%d], wr=[%d]",
                  (int)impl->data_.id_, (int)wantwr);

    impl->wantwr_ = wantwr;
    return 0;
}

static aug_id
cgetid_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return impl->data_.id_;
}

static char*
cgetname_(aug_chan* ob, char* dst, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    struct aug_endpoint ep;

    if (!aug_getpeername(impl->sticky_.md_, &ep)
        || !aug_endpointntop(&ep, dst, size))
        return NULL;

    return dst;
}

static aug_bool
cisready_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    unsigned short events;

    /* True if closed. */

    if (AUG_BADMD == impl->sticky_.md_)
        return AUG_TRUE;

    /* Channel-level events ready for processing. */

    if (readyevents_(impl))
        return AUG_TRUE;

    /* Or any SSL calls to be made. */

    events = aug_getsticky(&impl->sticky_);
    return isexcept_(impl, events)
        || isread_(impl, events)
        || iswrite_(impl, events);
}

static const struct aug_chanvtbl cvtbl_ = {
    ccast_,
    cretain_,
    crelease_,
    cclose_,
    cprocess_BI_,
    csetwantwr_,
    cgetid_,
    cgetname_,
    cisready_
};

static void*
scast_(aug_stream* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return cast_(impl, id);
}

static void
sretain_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    retain_(impl);
}

static void
srelease_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    release_(impl);
}

static aug_result
sshutdown_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);

    impl->shutdown_ = AUG_TRUE;

    /* If the output buffer is not empty, the shutdown call will be delayed
       until the remaining data has been written. */

    return bufempty_(&impl->outbuf_) ? shutwr_(impl, aug_tlerr) : 0;
}

static aug_rsize
sread_(aug_stream* ob, void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    if (ERROR_ == impl->state_) {

        /* Restore saved error. */

        aug_setctxerror(aug_tlx, impl->errinfo_.file_, impl->errinfo_.line_,
                        impl->errinfo_.src_, impl->errinfo_.num_,
                        impl->errinfo_.desc_);
        return -1;
    }

    /* Only return end once all data has been read from buffer. */

    if (ENDOF_ == impl->state_ && bufempty_(&impl->inbuf_))
        return 0;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (bufempty_(&impl->inbuf_)) {
        aug_setexcept(aug_tlx, AUG_EXBLOCK);
        return -1;
    }

    AUG_CTXDEBUG3(aug_tlx, "SSL: chan read from input buffer: id=[%d]",
                  (int)impl->data_.id_);

    ret = (ssize_t)readbuf_(&impl->inbuf_, buf, size);
    setmask_(impl);
    return ret;
}

static aug_rsize
sreadv_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    if (ERROR_ == impl->state_) {

        /* Restore saved error. */

        aug_setctxerror(aug_tlx, impl->errinfo_.file_, impl->errinfo_.line_,
                        impl->errinfo_.src_, impl->errinfo_.num_,
                        impl->errinfo_.desc_);
        return -1;
    }

    /* Only return end once all data has been read from buffer. */

    if (ENDOF_ == impl->state_ && bufempty_(&impl->inbuf_))
        return 0;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (bufempty_(&impl->inbuf_)) {
        aug_setexcept(aug_tlx, AUG_EXBLOCK);
        return -1;
    }

    AUG_CTXDEBUG3(aug_tlx, "SSL: chan readv from input buffer: id=[%d]",
                  (int)impl->data_.id_);

    ret = (ssize_t)readbufv_(&impl->inbuf_, iov, size);
    setmask_(impl);
    return ret;
}

static aug_rsize
swrite_(aug_stream* ob, const void* buf, size_t size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (buffull_(&impl->outbuf_)) {
        aug_setexcept(aug_tlx, AUG_EXBLOCK);
        return -1;
    }

    if (impl->shutdown_) {
#if !defined(_WIN32)
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, ESHUTDOWN);
#else /* _WIN32 */
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAESHUTDOWN);
#endif /* _WIN32 */
        return -1;
    }

    AUG_CTXDEBUG3(aug_tlx, "SSL: chan write to output buffer: id=[%d]",
                  (int)impl->data_.id_);

    ret = (ssize_t)writebuf_(&impl->outbuf_, buf, size);
    setmask_(impl);
    return ret;
}

static aug_rsize
swritev_(aug_stream* ob, const struct iovec* iov, int size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    ssize_t ret;

    /* Fail with EWOULDBLOCK is non-blocking operation would have blocked. */

    if (buffull_(&impl->outbuf_)) {
        aug_setexcept(aug_tlx, AUG_EXBLOCK);
        return -1;
    }

    if (impl->shutdown_) {
#if !defined(_WIN32)
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, ESHUTDOWN);
#else /* _WIN32 */
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAESHUTDOWN);
#endif /* _WIN32 */
        return -1;
    }

    AUG_CTXDEBUG3(aug_tlx, "SSL: chan writev to output buffer: id=[%d]",
                  (int)impl->data_.id_);

    ret = (ssize_t)writebufv_(&impl->outbuf_, iov, size);
    setmask_(impl);
    return ret;
}

static const struct aug_streamvtbl svtbl_ = {
    scast_,
    sretain_,
    srelease_,
    sshutdown_,
    sread_,
    sreadv_,
    swrite_,
    swritev_
};

static struct impl_*
createssl_(aug_mpool* mpool, aug_id id, aug_muxer_t muxer, aug_sd sd,
           aug_bool wantwr, aug_chandler* handler, struct ssl_ctx_st* sslctx)
{
    struct impl_* impl;
    SSL* ssl;
    BIO* bio;

    if (!(impl = aug_allocmem(mpool, sizeof(struct impl_))))
        return NULL;

    impl->chan_.vtbl_ = &cvtbl_;
    impl->chan_.impl_ = NULL;
    impl->stream_.vtbl_ = &svtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    /* Sticky event flags are used for edge-triggered interfaces. */

    if (aug_initsticky(&impl->sticky_, muxer, sd, 0) < 0) {
        aug_freemem(mpool, impl);
        return NULL;
    }

    impl->wantwr_ = wantwr;
    impl->data_.handler_ = handler;
    impl->data_.id_ = id;

    /* SSL state */

    ssl = SSL_new(sslctx);
    bio = BIO_new_socket((int)sd, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio, bio);
    SSL_set_app_data(ssl, &impl->data_);

    impl->ssl_ = ssl;
    clearbuf_(&impl->inbuf_);
    clearbuf_(&impl->outbuf_);
    impl->state_ = HANDSHAKE_;
    impl->except_ = 0;
    aug_clearerrinfo(&impl->errinfo_);
    impl->shutdown_ = AUG_FALSE;

    /* Initiate handshake mask. */

    setmask_(impl);

    aug_retain(mpool);
    aug_retain(handler);
    return impl;
}

AUGNET_API void
aug_setsslerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                  unsigned long err)
{
    aug_seterrinfo(errinfo, file, line, "ssl", err,
                   "%s: %s: %s",
                   ERR_lib_error_string(err),
                   ERR_func_error_string(err),
                   ERR_reason_error_string(err));

    /* Log those that are not set in errinfo record. */

    while ((err = ERR_get_error()))
        aug_ctxerror(aug_tlx, "%s: %s: %s",
                     ERR_lib_error_string(err),
                     ERR_func_error_string(err),
                     ERR_reason_error_string(err));
}

AUGNET_API aug_chan*
aug_createsslclient(aug_mpool* mpool, aug_id id, aug_muxer_t muxer,
                    aug_sd sd, aug_bool wantwr, aug_chandler* handler,
                    struct ssl_ctx_st* sslctx)
{
    struct impl_* impl = createssl_(mpool, id, muxer, sd, wantwr, handler,
                                    sslctx);
    if (!impl)
        return NULL;

    SSL_set_connect_state(impl->ssl_);
    return &impl->chan_;
}

AUGNET_API aug_chan*
aug_createsslserver(aug_mpool* mpool, aug_id id, aug_muxer_t muxer,
                    aug_sd sd, aug_bool wantwr, aug_chandler* handler,
                    struct ssl_ctx_st* sslctx)
{
    struct impl_* impl = createssl_(mpool, id, muxer, sd, wantwr, handler,
                                    sslctx);
    if (!impl)
        return NULL;

    SSL_set_accept_state(impl->ssl_);
    return &impl->chan_;
}

#endif /* WITH_SSL */
