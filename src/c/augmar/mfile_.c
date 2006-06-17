/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/mfile_.h"

static const char rcsid[] = "$Id:$";

#include "augmar/file_.h"
#include "augmar/format_.h"

#include "augsys/log.h"
#include "augsys/mmap.h"
#include "augsys/utility.h" /* aug_filesize() */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

struct aug_mfile_ {
    int fd_, flags_;
    size_t resvd_, size_;
    struct aug_mmap_* mmap_;
};

static int
toflags_(int from)
{
    int flags;
    switch (from & (AUG_RDONLY | AUG_WRONLY | AUG_RDWR)) {
    case AUG_RDONLY:
        flags = AUG_MMAPRD;
        break;
    case AUG_WRONLY:
    case AUG_RDWR:
        flags = AUG_MMAPRD | AUG_MMAPWR;
        break;
    default:
        flags = 0;
    }
    return flags;
}

static size_t
reserve_(size_t size)
{
    /* Given the size to be reserved, round up to the nearest page size. */

    size_t pagesize = aug_pagesize();
    size_t pages = size / pagesize;
    if (size % pagesize)
        ++pages;
    return pages * pagesize;
}

AUGMAR_EXTERN int
aug_closemfile_(aug_mfile_t mfile)
{
    int err = 0;
    assert(mfile);

    if (mfile->mmap_) {

        if (-1 == aug_freemmap(mfile->mmap_))
            err = errno;

        if (mfile->resvd_ > mfile->size_) {

            if (-1 == aug_truncatefile_(mfile->fd_, (off_t)mfile->size_)
                && !err)
                err = errno;
        }
    }

    if (-1 == aug_closefile_(mfile->fd_) && !err)
        err = errno;

    free(mfile);
    if (!err)
        return 0;

    errno = err;
    return -1;
}

AUGMAR_EXTERN aug_mfile_t
aug_openmfile_(const char* path, int flags, mode_t mode,
               size_t tail)
{
    int fd;
    size_t size;
    aug_mfile_t mfile;
    assert(path);

    /* Opening a file with the append flag set (where that file is to be
       memory mapped), does not seem to make much sense.  Therefore, the
       append flag if set is stripped off before the remaining flags are
       passed to the open file function. */

    if (-1 == (fd = aug_openfile_(path, flags & ~AUG_APPEND, mode)))
        return NULL;

    if (-1 == aug_filesize(fd, &size))
        return NULL;

    if (!(mfile = (aug_mfile_t)malloc(sizeof(struct aug_mfile_) + tail)))
        goto fail;

    mfile->fd_ = fd;
    mfile->flags_ = toflags_(flags);
    mfile->resvd_ = mfile->size_ = size;
    mfile->mmap_ = NULL;
    return mfile;

 fail:
    aug_closefile_(fd);
    return NULL;
}

AUGMAR_EXTERN void*
aug_mapmfile_(aug_mfile_t mfile, size_t size)
{
    assert(mfile);

    /* Check to see if a large enough area is already mapped. */

    if (mfile->mmap_ && mfile->mmap_->len_ >= size)
        goto done;

    if (mfile->resvd_ < size) {

        size_t resvd;

        /* If file has not been opened for writing, then the file cannot be
           extended. */

        if (!(mfile->flags_ & AUG_MMAPWR)) {

            errno = EINVAL;
            aug_error("file is not writable");
            return NULL;
        }

        resvd = reserve_(size);
        if (-1 == aug_extendfile_(mfile->fd_, resvd - mfile->size_))
            return NULL;

        mfile->resvd_ = resvd;
    }

    if (mfile->mmap_) {

        if (-1 == aug_remmap(mfile->mmap_, 0, size)) {

            int err = errno;
            aug_freemmap(mfile->mmap_);
            errno = err;
            mfile->mmap_ = NULL;
            return NULL;
        }
    } else {

        mfile->mmap_ = aug_createmmap(mfile->fd_, 0, size, mfile->flags_);
        if (!mfile->mmap_)
            return NULL;
    }

 done:
    if (size > mfile->size_)
        mfile->size_ = size;

    return mfile->mmap_->addr_;
}

AUGMAR_EXTERN int
aug_syncmfile_(aug_mfile_t mfile)
{
    assert(mfile);
    if (mfile->mmap_)
        return aug_syncmmap(mfile->mmap_);

    return aug_syncfile_(mfile->fd_);
}

AUGMAR_EXTERN int
aug_truncatemfile_(aug_mfile_t mfile, size_t size)
{
    assert(mfile);
    if (!(mfile->flags_ & AUG_MMAPWR)) {

        errno = EINVAL;
        aug_error("file is not writable");
        return -1;
    }
    mfile->size_ = size;
    return 0;
}

AUGMAR_EXTERN void*
aug_mfileaddr_(aug_mfile_t mfile)
{
    assert(mfile);
    if (!mfile->mmap_)
        return NULL;

    return mfile->mmap_->addr_;
}

AUGMAR_EXTERN size_t
aug_mfileresvd_(aug_mfile_t mfile)
{
    assert(mfile);
    return mfile->resvd_;
}

AUGMAR_EXTERN size_t
aug_mfilesize_(aug_mfile_t mfile)
{
    assert(mfile);
    return mfile->size_;
}

AUGMAR_EXTERN void*
aug_mfiletail_(aug_mfile_t mfile)
{
    assert(mfile);
    return (aug_byte_t*)mfile + sizeof(struct aug_mfile_);
}

