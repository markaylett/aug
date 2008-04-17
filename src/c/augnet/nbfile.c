/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/nbfile.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/extend.h"

#include "augsys/base.h"   /* struct aug_fdtype */
#include "augsys/socket.h" /* aug_shutdown() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <stdlib.h>        /* malloc() */

static struct aug_nbfile*
removenbfile_(struct aug_nbfile* nbfile)
{
    struct aug_nbfile* ret = nbfile;

    AUG_CTXDEBUG3(aug_tlx, "clearing io-event mask: fd=[%d]", nbfile->fd_);

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
    int ret = 0;

    AUG_CTXDEBUG3(aug_tlx, "nbfile close");

    if (!aug_resetnbfile(fd, &nbfile))
        return -1;

    if (!removenbfile_(&nbfile))
        ret = -1;

    if (!nbfile.base_->close_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
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
    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    if (!nbfile.base_->read_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                       AUG_MSG("aug_read() not supported"));
        return -1;
    }

    return nbfile.base_->read_(fd, buf, size);
}

static ssize_t
readv_(int fd, const struct iovec* iov, int size)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    if (!nbfile.base_->readv_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                       AUG_MSG("aug_readv() not supported"));
        return -1;
    }

    return nbfile.base_->readv_(fd, iov, size);
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    if (!nbfile.base_->write_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                       AUG_MSG("aug_write() not supported"));
        return -1;
    }

    return nbfile.base_->write_(fd, buf, len);
}

static ssize_t
writev_(int fd, const struct iovec* iov, int size)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    if (!nbfile.base_->writev_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                       AUG_MSG("aug_writev() not supported"));
        return -1;
    }

    return nbfile.base_->writev_(fd, iov, size);
}

static int
setnonblock_(int fd, int on)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;

    if (!nbfile.base_->setnonblock_) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
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
nbfilecb_(aug_object* ob, struct aug_nbfile* nbfile)
{
    int events = aug_fdevents(nbfile->nbfiles_->muxer_, nbfile->fd_);
    return events ? nbfile->cb_(ob, nbfile->fd_, (unsigned short)events) : 1;
}

static int
seteventmask_(struct aug_nbfile* nbfile, unsigned short mask)
{
    return aug_setfdeventmask(nbfile->nbfiles_->muxer_, nbfile->fd_, mask);
}

static int
eventmask_(struct aug_nbfile* nbfile)
{
    return aug_fdeventmask(nbfile->nbfiles_->muxer_, nbfile->fd_);
}

static int
events_(struct aug_nbfile* nbfile)
{
    return aug_fdevents(nbfile->nbfiles_->muxer_, nbfile->fd_);
}

static int
shutdown_(struct aug_nbfile* nbfile)
{
    return aug_shutdown(nbfile->fd_, SHUT_WR);
}

static const struct aug_nbtype nbtype_ = {
    nbfilecb_,
    seteventmask_,
    eventmask_,
    events_,
    shutdown_
};

static int
filecb_(aug_object* ob, int fd)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return 0; /* Not found so remove. */
    return nbfile.type_->filecb_(ob, &nbfile);
}

AUGNET_API aug_nbfiles_t
aug_createnbfiles(void)
{
    aug_nbfiles_t nbfiles = malloc(sizeof(struct aug_nbfiles_));
    if (!nbfiles) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }
    if (!(nbfiles->muxer_ = aug_createmuxer())) {
        free(nbfiles);
        return NULL;
    }
    AUG_INIT(&nbfiles->files_);
    nbfiles->nowait_ = 0;
    return nbfiles;
}

AUGNET_API int
aug_destroynbfiles(aug_nbfiles_t nbfiles)
{
    int ret = aug_destroyfiles(&nbfiles->files_);
    if (-1 == aug_destroymuxer(nbfiles->muxer_))
        ret = -1;
    free(nbfiles);
    return ret;
}

AUGNET_API int
aug_insertnbfile(aug_nbfiles_t nbfiles, int fd, aug_nbfilecb_t cb,
                 aug_object* ob)
{
    struct aug_nbfile nbfile;

    nbfile.nbfiles_ = nbfiles;
    nbfile.fd_ = fd;
    nbfile.cb_ = cb;
    if (!(nbfile.base_ = aug_setfdtype(fd, &fdtype_)))
        return -1;
    nbfile.type_ = &nbtype_;
    nbfile.ext_ = NULL;

    if (!aug_setnbfile(fd, &nbfile)
        || -1 == aug_insertfile(&nbfiles->files_, fd, filecb_, ob)) {

        /* On failure, restore original file type. */

        aug_setfdtype(fd, nbfile.base_);
        return -1;
    }

    return 0;
}

AUGNET_API int
aug_removenbfile(int fd)
{
    struct aug_nbfile nbfile;
    int ret = 0;

    AUG_CTXDEBUG3(aug_tlx, "aug_removenbfile()");

    if (!aug_resetnbfile(fd, &nbfile))
        return -1;

    if (!removenbfile_(&nbfile))
        ret = -1;

    if (!aug_setfdtype(nbfile.fd_, nbfile.base_))
        ret = -1;

    return ret;
}

AUGNET_API int
aug_foreachnbfile(aug_nbfiles_t nbfiles)
{
    AUG_CTXDEBUG3(aug_tlx, "aug_foreachnbfile()");
    return aug_foreachfile(&nbfiles->files_);
}

AUGNET_API int
aug_emptynbfiles(aug_nbfiles_t nbfiles)
{
    return AUG_EMPTY(&nbfiles->files_);
}

AUGNET_API int
aug_waitnbevents(aug_nbfiles_t nbfiles, const struct timeval* timeout)
{
    static const struct timeval nowait = { 0, 0 };
    int ret;

    AUG_CTXDEBUG3(aug_tlx, "aug_waitnbevents()");

    if (nbfiles->nowait_) {
        nbfiles->nowait_ = 0;
        ret = aug_waitfdevents(nbfiles->muxer_, &nowait)
            + 1; /* At least one. */
    } else
        ret = aug_waitfdevents(nbfiles->muxer_, timeout);

    return ret;
}

AUGNET_API int
aug_shutdownnbfile(int fd)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;
    return nbfile.type_->shutdown_(&nbfile);
}

AUGNET_API int
aug_setnbeventmask(int fd, unsigned short mask)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;
    return nbfile.type_->seteventmask_(&nbfile, mask);
}

AUGNET_API int
aug_nbeventmask(int fd)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;
    return nbfile.type_->eventmask_(&nbfile);
}

AUGNET_API int
aug_nbevents(int fd)
{
    struct aug_nbfile nbfile;
    if (!aug_getnbfile(fd, &nbfile))
        return -1;
    return nbfile.type_->events_(&nbfile);
}
