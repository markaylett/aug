/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/extend.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/lock.h"

#include <stdlib.h> /* malloc() */
#include <string.h> /* memcpy() */

#define BLOCKSIZE_ 256

static struct aug_nbfile* files_ = NULL;
static size_t size_ = 0;

static void
setbadfd_(const char* file, int line)
{
    aug_seterrinfo(NULL, file, line, AUG_SRCLOCAL, AUG_EINVAL,
                   AUG_MSG("invalid file descriptor"));
}

static void
zerofile(struct aug_nbfile* nbfile)
{
    nbfile->nbfiles_ = NULL;
    nbfile->fd_ = -1;
    nbfile->cb_ = NULL;
    nbfile->base_ = NULL;
    nbfile->type_ = NULL;
    nbfile->ext_ = NULL;
}

static void
zerofiles_(struct aug_nbfile* begin, size_t n)
{
    size_t i;
    for (i = 0; i < n; ++i)
        zerofile(begin + i);
}

static int
growfiles_(size_t size)
{
    struct aug_nbfile* files;
    size_t over = size % BLOCKSIZE_;

    /* Unless required size is an exact multiple of block size, round up to
       next block boundary. */

    if (0 != over)
        size += BLOCKSIZE_ - over;

    if (files_)
        files = realloc(files_, size * sizeof(struct aug_nbfile));
    else
        files = malloc(size * sizeof(struct aug_nbfile));

    if (!files) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return -1;
    }

    /* Initialise new file records. */

    zerofiles_(files + size_, size - size_);

    size_ = size;
    files_ = files;
    return 0;
}

static const struct aug_nbfile*
setnbfile_(int fd, const struct aug_nbfile* nbfile)
{
    if (size_ <= (size_t)fd && -1 == growfiles_(fd + 1))
        return NULL;

    memcpy(files_ + fd, nbfile, sizeof(*nbfile));
    return nbfile;
}

static struct aug_nbfile*
getnbfile_(int fd, struct aug_nbfile* nbfile)
{
    if (size_ <= (size_t)fd || !files_[fd].nbfiles_)
        return NULL;

    memcpy(nbfile, files_ + fd, sizeof(*nbfile));
    return nbfile;
}

static struct aug_nbfile*
resetnbfile_(int fd, struct aug_nbfile* nbfile)
{
    if (size_ <= (size_t)fd || !files_[fd].nbfiles_)
        return NULL;

    memcpy(nbfile, files_ + fd, sizeof(*nbfile));
    zerofile(files_ + fd);
    return nbfile;
}

AUGNET_API const struct aug_nbfile*
aug_setnbfile(int fd, const struct aug_nbfile* nbfile)
{
    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return NULL;
    }

    aug_lock();
    nbfile = setnbfile_(fd, nbfile);
    aug_unlock();

    return nbfile;
}

AUGNET_API struct aug_nbfile*
aug_getnbfile(int fd, struct aug_nbfile* nbfile)
{
    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return NULL;
    }

    aug_lock();
    nbfile = getnbfile_(fd, nbfile);
    aug_unlock();

    return nbfile;
}

AUGNET_API struct aug_nbfile*
aug_resetnbfile(int fd, struct aug_nbfile* nbfile)
{
    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        return NULL;
    }

    aug_lock();
    nbfile = resetnbfile_(fd, nbfile);
    aug_unlock();

    return nbfile;
}
