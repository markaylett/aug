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

#include <openssl/err.h>
#include <openssl/ssl.h>

#if !defined(_WIN32)
# define _get_osfhandle(x) x
#else /* _WIN32 */
# include <io.h>
#endif /* _WIN32 */

struct buf_ {
    char buf_[4096];
    char* rd_, * wr_;
};

enum sslstate_ {
    NORMAL,
    RDONWR,
    WRONRD,
    RDPEND,
    RDZERO,
    SSLERR
};

struct sslext_ {
    SSL* ssl_;
    struct buf_ inbuf_, outbuf_;
    enum sslstate_ state_;

    /* The event mask from the user's perspective. */

    unsigned short mask_;
};

static void
seterrinfo_(const char* file, int line, const char* s)
{
    unsigned long err = ERR_get_error();
    aug_seterrinfo(NULL, file, line, AUG_SRCSSL, err,
                   "%s: %s: %s: %s", s,
                   ERR_lib_error_string(err),
                   ERR_func_error_string(err),
                   ERR_reason_error_string(err));
}

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

static ssize_t
readbuf_(struct buf_* x, void* buf, size_t size)
{
    ssize_t ret = AUG_MIN(x->wr_ - x->rd_, size);
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

static ssize_t
writebuf_(struct buf_* x, const void* buf, size_t size)
{
    const char* end = x->buf_ + sizeof(x->buf_);
    ssize_t ret = AUG_MIN(end - x->wr_, size);
    if (ret) {

        /* Copy bytes to buffer at write pointer. */

        memcpy(x->wr_, buf, ret);
        x->wr_ += ret;
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
    struct sslext_* x = nbfile->ext_;
    int mask = 0;

    switch (x->state_) {
    case NORMAL:
        if (!bufempty_(&x->outbuf_))
            mask = AUG_FDEVENTRDWR;
        else
            mask = AUG_FDEVENTRD;
        break;
    case RDONWR:
        mask = AUG_FDEVENTWR;
        break;
    case WRONRD:
        mask = AUG_FDEVENTRD;
        break;
    case RDPEND:
    case RDZERO:
    case SSLERR:
        break;
    }

    return mask;
}

static int
close_(int fd)
{
    struct aug_nbfile nbfile;
    struct sslext_* x = aug_getnbfile(fd, &nbfile)->ext_;

    AUG_DEBUG2("clearing io-event mask: fd=[%d]", fd);
    aug_setfdeventmask(nbfile.nbfiles_->mplexer_, fd, 0);

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
    struct sslext_* x = aug_getnbfile(fd, &nbfile)->ext_;

    if (SSLERR == x->state_)
        return -1;

    if (RDZERO == x->state_ && bufempty_(&x->inbuf_))
        return 0;

    return readbuf_(&x->inbuf_, buf, size);
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    struct aug_nbfile nbfile;
    struct sslext_* x = aug_getnbfile(fd, &nbfile)->ext_;
    ssize_t ret = writebuf_(&x->outbuf_, buf, len);

    aug_setfdeventmask(nbfile.nbfiles_->mplexer_, nbfile.fd_,
                       realmask_(&nbfile));
    return ret;
}

static int
setnonblock_(int fd, int on)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);

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
    NULL,
    write_,
    NULL,
    setnonblock_
};

static void
readwrite_(struct sslext_* x, int events)
{
    int ret;

    /* Now check if there's data to read. */

    if ((events & AUG_FDEVENTRD)
        && (RDONWR == x->state_ || !buffull_(&x->inbuf_))) {

        x->state_ = NORMAL;

        ret = readssl_(x->ssl_, &x->inbuf_);
        switch (SSL_get_error(x->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_DEBUG2("SSL: %d bytes read to input buffer", ret);

            if ((ret = SSL_pending(x->ssl_))) {
                AUG_DEBUG2("SSL: %d bytes pending for immediate read", ret);
                x->state_ = RDPEND;
            }
            break;

        case SSL_ERROR_ZERO_RETURN:
            AUG_DEBUG2("SSL: end of data");
            x->state_ = RDZERO;
            return;

        case SSL_ERROR_WANT_READ:
            AUG_DEBUG2("SSL: read wants read");
            return;

            /* We get a WANT_WRITE if we're trying to rehandshake and we block
               on a write during that rehandshake.

               We need to wait on the socket to be writeable but reinitiate
               the read when it is. */

        case SSL_ERROR_WANT_WRITE:
            AUG_DEBUG2("SSL: read wants write");
            x->state_ = RDONWR;
            break;

        default:
            seterrinfo_(__FILE__, __LINE__, "SSL_read() failed");
            x->state_ = SSLERR;
            return;
        }
    }

    /* If the socket is writeable... */

    if ((events & AUG_FDEVENTWR)
        && (WRONRD == x->state_ || !bufempty_(&x->outbuf_))) {

        x->state_ = NORMAL;

        /* Try to write. */

        ret = writessl_(x->ssl_, &x->outbuf_);
        switch (SSL_get_error(x->ssl_, ret)) {
        case SSL_ERROR_NONE:
            AUG_DEBUG2("SSL: %d bytes written from output buffer", ret);
            break;

            /* We would have blocked. */

        case SSL_ERROR_WANT_WRITE:
            AUG_DEBUG2("SSL: write wants write");
            break;

            /* We get a WANT_READ if we're trying to rehandshake and we block
               on write during the current connection.

               We need to wait on the socket to be readable but reinitiate our
               write when it is. */

        case SSL_ERROR_WANT_READ:
            AUG_DEBUG2("SSL: write wants read");
            x->state_ = WRONRD;
            break;

            /* Some other error. */

        default:
            seterrinfo_(__FILE__, __LINE__, "SSL_write() failed");
            x->state_ = SSLERR;
            return;
        }
    }
}

static int
events_(struct aug_nbfile* nbfile);

static int
filecb_(const struct aug_var* var, struct aug_nbfile* nbfile,
        struct aug_files* files)
{
    struct sslext_* x = nbfile->ext_;
    int events = aug_fdevents(nbfile->nbfiles_->mplexer_, nbfile->fd_);
    int ret;

    switch (x->state_) {
    case NORMAL:
        break;
    case RDONWR:
        if (events & AUG_FDEVENTWR)
            events = AUG_FDEVENTRD;
        break;
    case WRONRD:
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
    }

    if (events)
        readwrite_(x, events);

    events = events_(nbfile);
    ret = events ? nbfile
        ->cb_(var, nbfile->fd_, (unsigned short)events, nbfile->nbfiles_) : 1;

    if (ret && !(events = realmask_(nbfile)))
        ++nbfile->nbfiles_->pending_;

    aug_setfdeventmask(nbfile->nbfiles_->mplexer_, nbfile->fd_,
                       (unsigned short)events);
    return ret;
}

static int
seteventmask_(struct aug_nbfile* nbfile, unsigned short mask)
{
    struct sslext_* x = nbfile->ext_;
    x->mask_ = mask;
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
    struct sslext_* x = nbfile->ext_;
    int events = 0;

    if ((x->mask_ & AUG_FDEVENTRD)
        && (!bufempty_(&x->inbuf_) || RDZERO == x->state_
            || SSLERR == x->state_))
        events |= AUG_FDEVENTRD;

    if ((x->mask_ & AUG_FDEVENTWR) && !buffull_(&x->outbuf_))
        events |= AUG_FDEVENTWR;

    AUG_DEBUG2("SSL: client events: mask=[%d], events=[%d]",
               x->mask_, events);
    return events;
}

static int
shutdown_(struct aug_nbfile* nbfile)
{
    struct sslext_* x = nbfile->ext_;
    int ret = SSL_shutdown(x->ssl_);

    if (ret <= 0) {
        seterrinfo_(__FILE__, __LINE__, "SSL_shutdown() failed");
        return -1;
    }
    return 0;
}

static const struct aug_nbtype nbtype_ = {
    filecb_,
    seteventmask_,
    eventmask_,
    events_,
    shutdown_
};

static struct sslext_*
createsslext_(aug_nbfiles_t nbfiles, int fd, SSL_CTX* ctx)
{
    struct sslext_* x;
    SSL* ssl = SSL_new(ctx);
    BIO* sbio = BIO_new_socket(_get_osfhandle(fd), BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);

    x = malloc(sizeof(struct sslext_));
    x->ssl_ = ssl;
    clearbuf_(&x->inbuf_);
    clearbuf_(&x->outbuf_);
    x->state_ = RDONWR;
    x->mask_ = aug_fdeventmask(nbfiles->mplexer_, fd);
    aug_setfdeventmask(nbfiles->mplexer_, fd, AUG_FDEVENTRDWR);

    return x;
}

static struct sslext_*
setsslext_(int fd, SSL_CTX* ctx)
{
    struct aug_nbfile nbfile;
    struct sslext_* x;

    aug_getnbfile(fd, &nbfile);
    aug_setfdtype(fd, &sslfdtype_);
    nbfile.type_ = &nbtype_;
    nbfile.ext_ = x = createsslext_(nbfile.nbfiles_, fd, ctx);
    aug_setnbfile(fd, &nbfile);

    return x;
}

AUGNET_API int
aug_setsslclient(int fd, void* ctx)
{
    struct sslext_* x = setsslext_(fd, ctx);
    SSL_set_connect_state(x->ssl_);
    return 0;
}

AUGNET_API int
aug_setsslserver(int fd, void* ctx)
{
    struct sslext_* x = setsslext_(fd, ctx);
    SSL_set_accept_state(x->ssl_);
    return 0;
}

#else /* !HAVE_OPENSSL_SSL_H */

#include "augsys/errinfo.h"

AUGNET_API int
aug_setsslclient(int fd, void* ctx)
{
    aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                   AUG_MSG("aug_setsslclient() not supported"));
    return -1;
}

AUGNET_API int
aug_setsslserver(int fd, void* ctx)
{
    aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                   AUG_MSG("aug_setsslserver() not supported"));
    return -1;
}

#endif /* !HAVE_OPENSSL_SSL_H */
