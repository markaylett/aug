/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/ssl.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if HAVE_OPENSSL_SSL_H

#include "augnet/extend.h"

#include "augsys/base.h"
#include "augsys/errinfo.h"
#include "augsys/log.h"
#include "augsys/socket.h" /* aug_shutdown() */
#include "augsys/uio.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

struct buf_ {
    char buf_[4096];
    char* rd_, * wr_;
};

enum sslstate_ {
    NORMAL,
    RDWANTRD,
    RDWANTWR,
    WRWANTRD,
    WRWANTWR,

    /* Bytes stored in SSL buffer awaiting SSL_read(). */

    RDPEND,

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
    size_t ret = AUG_MIN(x->wr_ - x->rd_, size);
    if (ret) {

        /* Copy bytes from buffer at read pointer. */

        memcpy(buf, x->rd_, ret);
        x->rd_ += ret;

        /* Clear if read has caught write. */

        if (bufempty_(x))
            clearbuf_(x);
    }
    return ret;
}

static size_t
writebuf_(struct buf_* x, const void* buf, size_t size)
{
    const char* end = x->buf_ + sizeof(x->buf_);
    size_t ret = AUG_MIN(end - x->wr_, size);
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

        if (n < iov->iov_len)
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

        if (n < iov->iov_len)
            break;
    }
    return ret;
}

static int
readssl_(SSL* ssl, struct buf_* x)
{
    /* Read bytes into buffer at write pointer. */

    const char* end = x->buf_ + sizeof(x->buf_);
    int ret = SSL_read(ssl, x->wr_, end - x->wr_);
    if (0 < ret)
        x->wr_ += ret;
    return ret;
}

static int
writessl_(SSL* ssl, struct buf_* x)
{
    /* Write bytes from buffer at read pointer. */

    int ret = SSL_write(ssl, x->rd_, x->wr_ - x->rd_);
    if (0 < ret) {
        x->rd_ += ret;

        /* Clear if read has caught write. */

        if (bufempty_(x))
            clearbuf_(x);
    }
    return ret;
}

static int
realmask_(struct aug_nbfile* nbfile)
{
    /* Calculate the real mask to be used by the mplexer. */

    struct sslext_* x = nbfile->ext_;
    int mask = 0;

    switch (x->state_) {
    case NORMAL:
        if (!bufempty_(&x->outbuf_))
            mask = AUG_FDEVENTRDWR;
        else
            mask = AUG_FDEVENTRD;
        break;
    case RDWANTRD:
    case WRWANTRD:
        mask = AUG_FDEVENTRD;
        break;
    case RDWANTWR:
    case WRWANTWR:
        mask = AUG_FDEVENTWR;
        break;
    case RDPEND:
    case RDZERO:
    case SSLERR:
        break;
    }

    AUG_DEBUG2("SSL: real mask: fd=[%d], mask=[%d]", nbfile->fd_, mask);
    return mask;
}

static int
userevents_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    int events = 0;

    if ((x->mask_ & AUG_FDEVENTRD)
        && (!bufempty_(&x->inbuf_) || RDZERO == x->state_
            || SSLERR == x->state_))
        events |= AUG_FDEVENTRD;

    if ((x->mask_ & AUG_FDEVENTWR) && !buffull_(&x->outbuf_))
        events |= AUG_FDEVENTWR;

    AUG_DEBUG2("SSL: user events: fd=[%d], mask=[%d], events=[%d]",
               nbfile->fd_, x->mask_, events);
    return events;
}

static void
updateevents_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;

    aug_setfdeventmask(nbfile->nbfiles_->mplexer_, nbfile->fd_,
                       realmask_(nbfile));

    if (userevents_(nbfile)
        || (RDPEND == x->state_ && !buffull_(&x->inbuf_)))
        nbfile->nbfiles_->nowait_ = 1;
}

static int
close_(int fd)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;

    if (!aug_resetnbfile(fd, &nbfile))
        return -1;

    x = nbfile.ext_;

    AUG_DEBUG2("clearing io-event mask: fd=[%d]", fd);
    aug_setfdeventmask(nbfile.nbfiles_->mplexer_, fd, 0);
    aug_removefile(&nbfile.nbfiles_->files_, fd);

    SSL_free(x->ssl_);
    free(x);

    if (!nbfile.base_->close_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_close() not supported"));
        return -1;
    }

    return nbfile.base_->close_(fd);
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

    if (!bufempty_(&x->inbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    AUG_DEBUG2("SSL: reading from user buffer: fd=[%d]", fd);

    ret = readbuf_(&x->inbuf_, buf, size);
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

    if (!bufempty_(&x->inbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    AUG_DEBUG2("SSL: reading from user buffer: fd=[%d]", fd);

    ret = readbufv_(&x->inbuf_, iov, size);
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

    AUG_DEBUG2("SSL: writing to user buffer: fd=[%d]", fd);

    /* Fail with EAGAIN is non-blocking operation would have blocked. */

    if (buffull_(&x->outbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    ret = writebuf_(&x->outbuf_, buf, len);
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

    AUG_DEBUG2("SSL: writing to user buffer: fd=[%d]", fd);

    /* Fail with EAGAIN is non-blocking operation would have blocked. */

    if (buffull_(&x->outbuf_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAGAIN);
        return -1;
    }

    ret = writebufv_(&x->outbuf_, iov, size);
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
readwrite_(struct sslext_* x, int events)
{
    int ret;

    /* Now check if there's data to read. */

    if ((events & AUG_FDEVENTRD)
        && (RDWANTRD == x->state_ || RDWANTWR == x->state_
            || !buffull_(&x->inbuf_))) {

        ret = readssl_(x->ssl_, &x->inbuf_);
        switch (SSL_get_error(x->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_DEBUG2("SSL: %d bytes read to input buffer", ret);
            if ((ret = SSL_pending(x->ssl_))) {
                AUG_DEBUG2("SSL: %d bytes pending for immediate read", ret);
                x->state_ = RDPEND;
                goto done;
            }
            x->state_ = NORMAL;
            break;

        case SSL_ERROR_ZERO_RETURN:
            AUG_DEBUG2("SSL: end of data");
            if (!x->shutdown_) {
                AUG_DEBUG2("SSL: shutting-down", ret);
                SSL_shutdown(x->ssl_);
            }
            x->state_ = RDZERO;
            goto done;

        case SSL_ERROR_WANT_READ:
            AUG_DEBUG2("SSL: read wants read");
            x->state_ = RDWANTRD;
            goto done;

            /* We get a WANT_WRITE if we're trying to rehandshake and we block
               on a write during that rehandshake.

               We need to wait on the socket to be writeable but reinitiate
               the read when it is. */

        case SSL_ERROR_WANT_WRITE:
            AUG_DEBUG2("SSL: read wants write");
            x->state_ = RDWANTWR;
            goto done;

        default:
            aug_setsslerrinfo(NULL, __FILE__, __LINE__, ERR_get_error());
            x->state_ = SSLERR;
            goto done;
        }
    }

    /* If the socket is writeable... */

    if ((events & AUG_FDEVENTWR)
        && (WRWANTRD == x->state_ || WRWANTWR == x->state_
            || !bufempty_(&x->outbuf_))) {

        /* Try to write. */

        ret = writessl_(x->ssl_, &x->outbuf_);
        switch (SSL_get_error(x->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_DEBUG2("SSL: %d bytes written from output buffer", ret);
            x->state_ = NORMAL;
            break;

            /* We would have blocked. */

        case SSL_ERROR_WANT_WRITE:
            AUG_DEBUG2("SSL: write wants write");
            x->state_ = WRWANTWR;
            break;

            /* We get a WANT_READ if we're trying to rehandshake and we block
               on write during the current connection.

               We need to wait on the socket to be readable but reinitiate our
               write when it is. */

        case SSL_ERROR_WANT_READ:
            AUG_DEBUG2("SSL: write wants read");
            x->state_ = WRWANTRD;
            break;

            /* Some other error. */

        default:
            aug_setsslerrinfo(NULL, __FILE__, __LINE__, ERR_get_error());
            x->state_ = SSLERR;
            break;
        }
    }
 done:
    return;
}

static int
nbfilecb_(const struct aug_var* var, struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    int events = aug_fdevents(nbfile->nbfiles_->mplexer_, nbfile->fd_);
    int ret;

    /* Transform events. */

    switch (x->state_) {
    case RDWANTWR:
        if (events & AUG_FDEVENTWR)
            events = AUG_FDEVENTRD;
        break;
    case WRWANTRD:
        if (events & AUG_FDEVENTRD)
            events = AUG_FDEVENTWR;
        break;
    case RDPEND:
        if (buffull_(&x->inbuf_))
            events = 0;
        else
            events |= AUG_FDEVENTRD;
        break;
    case RDZERO:
    case SSLERR:
        events = 0;
        break;
    default:
        break;
    }

    if (events)
        readwrite_(x, events);

    events = userevents_(nbfile);
    ret = events ? nbfile->cb_(var, nbfile->fd_, events) : 1;

    updateevents_(nbfile);
    return ret;
}

static int
seteventmask_(struct aug_nbfile* nbfile, unsigned short mask)
{
    struct sslext_* x = nbfile->ext_;

    AUG_DEBUG2("SSL: setting event mask: fd=[%d], mask=[%d]",
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
    int ret = SSL_shutdown(x->ssl_);
    aug_shutdown(nbfile->fd_, SHUT_WR);
    x->shutdown_ = 1;

    if (ret <= 0) {
        aug_setsslerrinfo(NULL, __FILE__, __LINE__, ERR_get_error());
        return -1;
    }
    return 0;
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
    x->state_ = RDWANTWR;
    x->shutdown_ = 0;
    x->mask_ = aug_fdeventmask(nbfiles->mplexer_, fd);
    aug_setfdeventmask(nbfiles->mplexer_, fd, AUG_FDEVENTRDWR);

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
aug_setsslclient(int fd, void* ssl)
{
    if (!setsslext_(fd, ssl))
        return -1;

    SSL_set_connect_state(ssl);
    return 0;
}

AUGNET_API int
aug_setsslserver(int fd, void* ssl)
{
    if (!setsslext_(fd, ssl))
        return -1;

    SSL_set_accept_state(ssl);
    return 0;
}

#endif /* HAVE_OPENSSL_SSL_H */
