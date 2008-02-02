/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/ssl.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_SSL

# include "augnet/extend.h"

# include "augsys/base.h"
# include "augsys/errinfo.h"
# include "augsys/log.h"
# include "augsys/socket.h" /* aug_shutdown() */
# include "augsys/uio.h"

# include <openssl/err.h>
# include <openssl/ssl.h>

# include <string.h>        /* memcpy() */

# define SSLREAD_  0x01
# define SSLWRITE_ 0x02

struct buf_ {
    char buf_[4096];
    char* rd_, * wr_;
};

enum sslstate_ {
    NORMAL,

    /* Bytes stored in SSL buffer awaiting SSL_read(). */

    RDPEND,
    RDWANTRD,
    RDWANTWR,
    WRWANTRD,
    WRWANTWR,

    /* End of data. */

    RDZERO,
    SSLERR
};

struct sslext_ {
    SSL* ssl_;
    struct buf_ inbuf_, outbuf_;
    enum sslstate_ state_;
    int shutdown_;

    /* The event mask from the user's perspective. */

    unsigned short mask_;
};

static int
bufempty_(struct buf_* x)
{
    /* Return true if the buffer is empty. */

    return x->rd_ == x->wr_;
}

static int
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

        /* Clear if read pointer has caught write. */

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
shutwr_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    int ret;

    AUG_DEBUG3("SSL: shutdown");
    ret = SSL_shutdown(x->ssl_);
    aug_shutdown(nbfile->fd_, SHUT_WR);

    if (ret < 0) {
        aug_setsslerrinfo(NULL, __FILE__, __LINE__, ERR_get_error());
        return -1;
    }
    return 0;
}

static int
sslread_(SSL* ssl, struct buf_* x)
{
    /* Read bytes into buffer at write pointer. */

    const char* end = x->buf_ + sizeof(x->buf_);
    int n = (int)(end - x->wr_);
    if (n) {
        AUG_DEBUG3("SSL: ssl read to input buffer");
        n = SSL_read(ssl, x->wr_, n);
        if (0 < n)
            x->wr_ += n;
    } else {

        /* Input buffer is full, try handshake. */

        AUG_DEBUG3("SSL: handshake without read");
        n = SSL_do_handshake(ssl);
    }
    return n;
}

static int
sslwrite_(SSL* ssl, struct buf_* x)
{
    /* Write bytes from buffer at read pointer. */

    int n = (int)(x->wr_ - x->rd_);
    if (n) {
        AUG_DEBUG3("SSL: ssl write from output buffer");
        n = SSL_write(ssl, x->rd_, n);
        if (0 < n) {
            x->rd_ += n;

            /* Clear if read pointer has caught write. */

            if (bufempty_(x))
                clearbuf_(x);
        }
    } else {

        /* Output buffer is empty, try handshake. */

        AUG_DEBUG3("SSL: handshake without write");
        n = SSL_do_handshake(ssl);
    }
    return n;
}

static int
realmask_(struct aug_nbfile* nbfile)
{
    /* Calculate the real mask to be used by the muxer. */

    struct sslext_* x = nbfile->ext_;
    int mask = 0;

    switch (x->state_) {
    case NORMAL:
        if (!bufempty_(&x->outbuf_))
            mask = AUG_FDEVENTRDWR;
        else
            mask = AUG_FDEVENTRD;
        break;
    case RDPEND:
        break;
    case RDWANTRD:
    case WRWANTRD:
        mask = AUG_FDEVENTRD;
        break;
    case RDWANTWR:
    case WRWANTWR:
        mask = AUG_FDEVENTWR;
        break;
    case RDZERO:
    case SSLERR:
        break;
    }

    return mask;
}

static int
userevents_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    int events = 0;

    /* End of data and errors are communicated via aug_read(). */

    if ((x->mask_ & AUG_FDEVENTRD)
        && (!bufempty_(&x->inbuf_) || RDZERO == x->state_
            || SSLERR == x->state_))
        events |= AUG_FDEVENTRD;

    if ((x->mask_ & AUG_FDEVENTWR) && !buffull_(&x->outbuf_))
        events |= AUG_FDEVENTWR;

    return events;
}

static void
updateevents_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    int real, user;

    aug_setfdeventmask(nbfile->nbfiles_->muxer_, nbfile->fd_,
                       (real = realmask_(nbfile)));

    if ((user = userevents_(nbfile))
        || (RDPEND == x->state_ && !buffull_(&x->inbuf_)))
        nbfile->nbfiles_->nowait_ = 1;

    AUG_DEBUG3("SSL: events: fd=[%d], realmask=[%d], usermask=[%d],"
               " userevents=[%d]", nbfile->fd_, real, x->mask_, user);
}

static struct aug_nbfile*
removenbfile_(struct aug_nbfile* nbfile)
{
    struct aug_nbfile* ret = nbfile;

    AUG_DEBUG3("clearing io-event mask: fd=[%d]", nbfile->fd_);

    if (-1 == aug_setfdeventmask(nbfile->nbfiles_->muxer_, nbfile->fd_, 0))
        ret = NULL;

    if (-1 == aug_removefile(&nbfile->nbfiles_->files_, nbfile->fd_))
        ret = NULL;

    return ret;
}

static int
close_(int fd)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;
    int ret = 0;

    AUG_DEBUG3("nbfile close");

    if (!aug_resetnbfile(fd, &nbfile))
        return -1;

    if (!removenbfile_(&nbfile))
        ret = -1;

    x = nbfile.ext_;
    SSL_free(x->ssl_);
    free(x);

    if (!nbfile.base_->close_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_close() not supported"));
        return -1;
    }

    if (-1 == nbfile.base_->close_(fd))
        ret = -1;

    return ret;
}

static ssize_t
read_(int fd, void* buf, size_t size)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;
    ssize_t ret;

    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    x = nbfile.ext_;

    if (SSLERR == x->state_)
        return -1;

    /* Only return end once all data has been read from buffer. */

    if (RDZERO == x->state_ && bufempty_(&x->inbuf_))
        return 0;

    /* Fail with EAGAIN is non-blocking operation would have blocked. */

    if (bufempty_(&x->inbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    AUG_DEBUG3("SSL: user read from input buffer: fd=[%d]", fd);

    ret = (ssize_t)readbuf_(&x->inbuf_, buf, size);
    updateevents_(&nbfile);
    return ret;
}

static ssize_t
readv_(int fd, const struct iovec* iov, int size)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;
    ssize_t ret;

    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    x = nbfile.ext_;

    if (SSLERR == x->state_)
        return -1;

    /* Only return end once all data has been read from buffer. */

    if (RDZERO == x->state_ && bufempty_(&x->inbuf_))
        return 0;

    /* Fail with EAGAIN is non-blocking operation would have blocked. */

    if (bufempty_(&x->inbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    AUG_DEBUG3("SSL: user readv from input buffer: fd=[%d]", fd);

    ret = (ssize_t)readbufv_(&x->inbuf_, iov, size);
    updateevents_(&nbfile);
    return ret;
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;
    ssize_t ret;

    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    x = nbfile.ext_;

    /* Fail with EAGAIN is non-blocking operation would have blocked. */

    if (buffull_(&x->outbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    if (x->shutdown_) {
#if !defined(_WIN32)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ESHUTDOWN);
#else /* _WIN32 */
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAESHUTDOWN);
#endif /* _WIN32 */
        return -1;
    }

    AUG_DEBUG3("SSL: user write to output buffer: fd=[%d]", fd);

    ret = (ssize_t)writebuf_(&x->outbuf_, buf, len);
    updateevents_(&nbfile);
    return ret;
}

static ssize_t
writev_(int fd, const struct iovec* iov, int size)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;
    ssize_t ret;

    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    x = nbfile.ext_;

    /* Fail with EAGAIN is non-blocking operation would have blocked. */

    if (buffull_(&x->outbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    if (x->shutdown_) {
#if !defined(_WIN32)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ESHUTDOWN);
#else /* _WIN32 */
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAESHUTDOWN);
#endif /* _WIN32 */
        return -1;
    }

    AUG_DEBUG3("SSL: user writev to output buffer: fd=[%d]", fd);

    ret = (ssize_t)writebufv_(&x->outbuf_, iov, size);
    updateevents_(&nbfile);
    return ret;
}

static int
setnonblock_(int fd, int on)
{
    struct aug_nbfile nbfile;

    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    if (!nbfile.base_->setnonblock_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_setnonblock() not supported"));
        return -1;
    }

    return nbfile.base_->setnonblock_(fd, on);
}

static const struct aug_fdtype sslfdtype_ = {
    close_,
    read_,
    readv_,
    write_,
    writev_,
    setnonblock_
};

static void
readwrite_(struct aug_nbfile* nbfile, int rw)
{
    struct sslext_* x = nbfile->ext_;
    int ret;

    if (rw & SSLREAD_) {

        /* Perform read operation.  If input buffer is full, sslread_() will
           perform SSL_do_handshake(). */

        ret = sslread_(x->ssl_, &x->inbuf_);
        switch (SSL_get_error(x->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_DEBUG3("SSL: %d bytes read to input buffer", ret);
            if ((ret = SSL_pending(x->ssl_))) {
                AUG_DEBUG3("SSL: %d bytes pending for immediate read", ret);
                x->state_ = RDPEND;
                goto done;
            }
            x->state_ = NORMAL;
            break;
        case SSL_ERROR_ZERO_RETURN:
            AUG_DEBUG3("SSL: end of data");
            if (!x->shutdown_) {
                AUG_DEBUG3("SSL: shutting-down", ret);
                SSL_shutdown(x->ssl_);
            }
            x->state_ = RDZERO;
            goto done;
        case SSL_ERROR_WANT_READ:
            AUG_DEBUG3("SSL: read wants read");
            x->state_ = RDWANTRD;
            goto done;
        case SSL_ERROR_WANT_WRITE:
            AUG_DEBUG3("SSL: read wants write");
            x->state_ = RDWANTWR;
            goto done;
        default:
            aug_setsslerrinfo(NULL, __FILE__, __LINE__, ERR_get_error());
            x->state_ = SSLERR;
            goto done;
        }
    }

    if (rw & SSLWRITE_) {

        /* Perform write operation.  If output buffer is empty, sslwrite_()
           will perform SSL_do_handshake(). */

        ret = sslwrite_(x->ssl_, &x->outbuf_);
        switch (SSL_get_error(x->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_DEBUG3("SSL: %d bytes written from output buffer", ret);
            x->state_ = NORMAL;
            break;
        case SSL_ERROR_WANT_WRITE:
            AUG_DEBUG3("SSL: write wants write");
            x->state_ = WRWANTWR;
            break;
        case SSL_ERROR_WANT_READ:
            AUG_DEBUG3("SSL: write wants read");
            x->state_ = WRWANTRD;
            break;
        default:
            aug_setsslerrinfo(NULL, __FILE__, __LINE__, ERR_get_error());
            x->state_ = SSLERR;
            break;
        }

        /* If shutdown is pending and output buffer is now empty, do
           shutdown. */

        if (x->shutdown_ && bufempty_(&x->outbuf_) && -1 == shutwr_(nbfile))
            x->state_ = SSLERR;
    }
 done:
    return;
}

static int
nbfilecb_(aug_object* ob, struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    int events = aug_fdevents(nbfile->nbfiles_->muxer_, nbfile->fd_);
    int ret, rw = 0;

    /* Determine which SSL operations are to be performed. */

    switch (x->state_) {
    case RDPEND:
        if (!buffull_(&x->inbuf_)) {

            /* Data is available for immediate read to input buffer. */
            break;
        }

        /* Fall through to normal case. */

    case NORMAL:
        if (events & AUG_FDEVENTRD)
            rw = SSLREAD_;
        if (events & AUG_FDEVENTWR)
            rw |= SSLWRITE_;
        break;
    case RDWANTRD:
        if (events & AUG_FDEVENTRD)
            rw = SSLREAD_;
        break;
    case RDWANTWR:
        if (events & AUG_FDEVENTWR)
            rw = SSLREAD_;
        break;
    case WRWANTRD:
        if (events & AUG_FDEVENTRD)
            rw = SSLWRITE_;
        break;
    case WRWANTWR:
        if (events & AUG_FDEVENTWR)
            rw = SSLWRITE_;
        break;
    case RDZERO:
    case SSLERR:
        break;
    }

    if (rw)
        readwrite_(nbfile, rw);
    else
        AUG_DEBUG3("SSL: readwrite_() skipped");

    if ((events = userevents_(nbfile))) {

        AUG_DEBUG3("SSL: nbfilecb_(): fd=[%d], events=[%d]",
                   nbfile->fd_, events);

        /* Callback may close file, ensure that it is still available after
           callback returns. */

        aug_retainfd(nbfile->fd_);
        if ((ret = nbfile->cb_(ob, nbfile->fd_, events))) {

            /* No need to update events if file is being removed - indicated
               by false return. */

            updateevents_(nbfile);
        }
        aug_releasefd(nbfile->fd_);

    } else {
        AUG_DEBUG3("SSL: nbfilecb_() skipped");
        ret = 1;
    }

    return ret;
}

static int
seteventmask_(struct aug_nbfile* nbfile, unsigned short mask)
{
    struct sslext_* x = nbfile->ext_;

    AUG_DEBUG3("SSL: setting event mask: fd=[%d], mask=[%d]",
               nbfile->fd_, (int)mask);

    x->mask_ = mask;
    updateevents_(nbfile);
    return 0;
}

static int
eventmask_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    return x->mask_;
}

static int
events_(struct aug_nbfile* nbfile)
{
    return userevents_(nbfile);
}

static int
shutdown_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    x->shutdown_ = 1;

    /* If the output buffer is not empty, the shutdown call will be delayed
       until the remaining data has been written. */

    return bufempty_(&x->outbuf_) ? shutwr_(nbfile) : 0;
}

static const struct aug_nbtype nbtype_ = {
    nbfilecb_,
    seteventmask_,
    eventmask_,
    events_,
    shutdown_
};

static struct sslext_*
createsslext_(aug_nbfiles_t nbfiles, int fd, SSL* ssl)
{
    struct sslext_* x = malloc(sizeof(struct sslext_));
    if (!x) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    x->ssl_ = ssl;
    clearbuf_(&x->inbuf_);
    clearbuf_(&x->outbuf_);
    x->state_ = NORMAL;
    x->shutdown_ = 0;
    x->mask_ = aug_fdeventmask(nbfiles->muxer_, fd);
    aug_setfdeventmask(nbfiles->muxer_, fd, AUG_FDEVENTRDWR);

    return x;
}

static struct sslext_*
setsslext_(int fd, SSL* ssl)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;

    if (!aug_getnbfile(fd, &nbfile)
        || !(x = createsslext_(nbfile.nbfiles_, fd, ssl)))
        return NULL;

    aug_setfdtype(fd, &sslfdtype_);
    nbfile.type_ = &nbtype_;
    nbfile.ext_ = x;
    aug_setnbfile(fd, &nbfile);

    return x;
}

AUGNET_API void
aug_setsslerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                  unsigned long err)
{
    aug_seterrinfo(errinfo, file, line, AUG_SRCSSL, err,
                   "%s: %s: %s",
                   ERR_lib_error_string(err),
                   ERR_func_error_string(err),
                   ERR_reason_error_string(err));

    /* Log those that are not set in errinfo record. */

    while ((err = ERR_get_error()))
        aug_error("%s: %s: %s",
                  ERR_lib_error_string(err),
                  ERR_func_error_string(err),
                  ERR_reason_error_string(err));
}

AUGNET_API int
aug_setsslclient(int fd, struct ssl_st* ssl)
{
    struct sslext_* x = setsslext_(fd, ssl);
    if (!x)
        return -1;

    /* Write client-initiated handshake. */

    x->state_ = WRWANTWR;

    SSL_set_connect_state(ssl);
    return 0;
}

AUGNET_API int
aug_setsslserver(int fd, struct ssl_st* ssl)
{
    struct sslext_* x = setsslext_(fd, ssl);
    if (!x)
        return -1;

    /* Read client-initiated handshake. */

    x->state_ = RDWANTRD;

    SSL_set_accept_state(ssl);
    return 0;
}

#endif /* ENABLE_SSL */
