/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/mfile_.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augmar/file_.h"
#include "augmar/format_.h"

#include "augsys/errinfo.h"
#include "augsys/log.h"
#include "augsys/mmap.h"
#include "augsys/utility.h" /* aug_filesize() */

#include <assert.h>
#include <errno.h>          /* ENOMEM */
#include <stdlib.h>

struct aug_mfile_ {
    int fd_, flags_;
    unsigned resvd_, size_;
    struct aug_mmap* mmap_;
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

static unsigned
reserve_(unsigned size)
{
    /* Given the size to be reserved, round up to the nearest page size. */

    unsigned pagesize = aug_pagesize();
    unsigned pages = size / pagesize;
    if (size % pagesize)
        ++pages;
    return pages * pagesize;
}

AUG_EXTERNC int
aug_closemfile_(aug_mfile_t mfile)
{
    int ret = 0;
    assert(mfile);

    if (mfile->mmap_) {

        if (-1 == aug_destroymmap(mfile->mmap_))
            ret = -1;

        if (mfile->resvd_ > mfile->size_) {

            if (-1 == aug_truncatefile_(mfile->fd_, (off_t)mfile->size_))
                ret = -1;
        }
    }

    if (-1 == aug_closefile_(mfile->fd_))
        ret = -1;

    free(mfile);
    return ret;
}

AUG_EXTERNC aug_mfile_t
aug_openmfile_(const char* path, int flags, mode_t mode,
               unsigned tail)
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

    if (!(mfile = (aug_mfile_t)malloc(sizeof(struct aug_mfile_) + tail))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        goto fail;
    }

    mfile->fd_ = fd;
    mfile->flags_ = toflags_(flags);
    mfile->resvd_ = mfile->size_ = (unsigned)size;
    mfile->mmap_ = NULL;
    return mfile;

 fail:
    aug_closefile_(fd);
    return NULL;
}

AUG_EXTERNC void*
aug_mapmfile_(aug_mfile_t mfile, unsigned size)
{
    assert(mfile);

    /* Check to see if a large enough area is already mapped. */

    if (mfile->mmap_ && mfile->mmap_->len_ >= size)
        goto done;

    if (mfile->resvd_ < size) {

        unsigned resvd;

        /* If file has not been opened for writing, then the file cannot be
           extended. */

        if (!(mfile->flags_ & AUG_MMAPWR)) {

            aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EACCES,
                           AUG_MSG("file is not writable"));
            return NULL;
        }

        resvd = reserve_(size);
        if (-1 == aug_extendfile_(mfile->fd_, resvd - mfile->size_))
            return NULL;

        mfile->resvd_ = resvd;
    }

    if (mfile->mmap_) {

        if (-1 == aug_remmap(mfile->mmap_, 0, size)) {

            aug_destroymmap(mfile->mmap_);
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

AUG_EXTERNC int
aug_syncmfile_(aug_mfile_t mfile)
{
    assert(mfile);
    if (mfile->mmap_)
        return aug_syncmmap(mfile->mmap_);

    return aug_syncfile_(mfile->fd_);
}

AUG_EXTERNC int
aug_truncatemfile_(aug_mfile_t mfile, unsigned size)
{
    assert(mfile);
    if (!(mfile->flags_ & AUG_MMAPWR)) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EACCES,
                       AUG_MSG("file is not writable"));
        return -1;
    }
    mfile->size_ = size;
    return 0;
}

AUG_EXTERNC void*
aug_mfileaddr_(aug_mfile_t mfile)
{
    assert(mfile);
    if (!mfile->mmap_)
        return NULL;

    return mfile->mmap_->addr_;
}

AUG_EXTERNC unsigned
aug_mfileresvd_(aug_mfile_t mfile)
{
    assert(mfile);
    return mfile->resvd_;
}

AUG_EXTERNC unsigned
aug_mfilesize_(aug_mfile_t mfile)
{
    assert(mfile);
    return mfile->size_;
}

AUG_EXTERNC void*
aug_mfiletail_(aug_mfile_t mfile)
{
    assert(mfile);
    return (char*)mfile + sizeof(struct aug_mfile_);
}

