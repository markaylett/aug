/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/nbfile.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augnet/extend.h"

#include "augsys/base.h"
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/socket.h" /* aug_shutdown() */

static int
close_(int fd)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);

    AUG_DEBUG2("clearing io-event mask: fd=[%d]", fd);
    aug_setfdeventmask(nbfile.nbfiles_->mplexer_, fd, 0);

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
    aug_getnbfile(fd, &nbfile);

    if (!nbfile.base_->read_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_read() not supported"));
        return -1;
    }

    return nbfile.base_->read_(fd, buf, size);
}

static ssize_t
readv_(int fd, const struct iovec* iov, int size)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);

    if (!nbfile.base_->readv_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_readv() not supported"));
        return -1;
    }

    return nbfile.base_->readv_(fd, iov, size);
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);

    if (!nbfile.base_->write_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_write() not supported"));
        return -1;
    }

    return nbfile.base_->write_(fd, buf, len);
}

static ssize_t
writev_(int fd, const struct iovec* iov, int size)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);

    if (!nbfile.base_->writev_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_writev() not supported"));
        return -1;
    }

    return nbfile.base_->writev_(fd, iov, size);
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

static const struct aug_fdtype fdtype_ = {
    close_,
    read_,
    readv_,
    write_,
    writev_,
    setnonblock_
};

static int
filecb_(const struct aug_var* var, struct aug_nbfile* nbfile,
        struct aug_files* files)
{
    int events = aug_fdevents(nbfile->nbfiles_->mplexer_, nbfile->fd_);
    return events ? nbfile
        ->cb_(var, nbfile->fd_, (unsigned short)events, nbfile->nbfiles_) : 1;
}

static int
seteventmask_(struct aug_nbfile* nbfile, unsigned short mask)
{
    return aug_setfdeventmask(nbfile->nbfiles_->mplexer_, nbfile->fd_, mask);
}

static int
eventmask_(struct aug_nbfile* nbfile)
{
    return aug_fdeventmask(nbfile->nbfiles_->mplexer_, nbfile->fd_);
}

static int
events_(struct aug_nbfile* nbfile)
{
    return aug_fdevents(nbfile->nbfiles_->mplexer_, nbfile->fd_);
}

static int
shutdown_(struct aug_nbfile* nbfile)
{
    return aug_shutdown(nbfile->fd_, SHUT_WR);
}

static const struct aug_nbtype nbtype_ = {
    filecb_,
    seteventmask_,
    eventmask_,
    events_,
    shutdown_
};

static int
cb_(const struct aug_var* var, int fd, struct aug_files* files)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);
    return nbfile.type_->filecb_(var, &nbfile, files);
}

AUGNET_API aug_nbfiles_t
aug_createnbfiles(void)
{
    aug_nbfiles_t nbfiles = malloc(sizeof(struct aug_nbfiles_));
    if (!nbfiles) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }
    if (!(nbfiles->mplexer_ = aug_createmplexer())) {
        free(nbfiles);
        return NULL;
    }
    AUG_INIT(&nbfiles->files_);
    nbfiles->pending_ = 0;
    return nbfiles;
}

AUGNET_API int
aug_destroynbfiles(aug_nbfiles_t nbfiles)
{
    int ret = aug_destroyfiles(&nbfiles->files_);
    if (-1 == aug_destroymplexer(nbfiles->mplexer_))
        ret = -1;
    free(nbfiles);
    return ret;
}

AUGNET_API int
aug_insertnbfile(aug_nbfiles_t nbfiles, int fd, aug_nbfilecb_t cb,
                 const struct aug_var* var)
{
    struct aug_nbfile nbfile;

    nbfile.nbfiles_ = nbfiles;
    nbfile.fd_ = fd;
    nbfile.cb_ = cb;
    nbfile.base_ = aug_getfdtype(fd);
    nbfile.type_ = &nbtype_;
    nbfile.ext_ = NULL;

    aug_setnbfile(fd, &nbfile);

    if (-1 == aug_setfdtype(fd, &fdtype_))
        return -1;

    return aug_insertfile(&nbfiles->files_, fd, cb_, var);
}

AUGNET_API int
aug_removenbfile(int fd)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);
    return aug_removefile(&nbfile.nbfiles_->files_, fd);
}

AUGNET_API int
aug_foreachnbfile(aug_nbfiles_t nbfiles)
{
    nbfiles->pending_ = 0;
    return aug_foreachfile(&nbfiles->files_);
}

AUGNET_API int
aug_waitnbevents(aug_nbfiles_t nbfiles, const struct timeval* timeout)
{
    return nbfiles->pending_ ? nbfiles->pending_
        : aug_waitfdevents(nbfiles->mplexer_, timeout);
}

AUGNET_API int
aug_emptynbfiles(aug_nbfiles_t nbfiles)
{
    return AUG_EMPTY(&nbfiles->files_);
}

AUGNET_API int
aug_shutdownnbfile(int fd)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);
    return nbfile.type_->shutdown_(&nbfile);
}

AUGNET_API int
aug_setnbeventmask(int fd, unsigned short mask)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);
    return nbfile.type_->seteventmask_(&nbfile, mask);
}

AUGNET_API int
aug_nbeventmask(int fd)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);
    return nbfile.type_->eventmask_(&nbfile);
}

AUGNET_API int
aug_nbevents(int fd)
{
    struct aug_nbfile nbfile;
    aug_getnbfile(fd, &nbfile);
    return nbfile.type_->events_(&nbfile);
}
