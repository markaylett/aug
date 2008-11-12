/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/mfile_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/format_.h"

#include "augsys/mmap.h"
#include "augsys/unistd.h" /* aug_fsize() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"

#include <assert.h>
#include <stdlib.h>

struct aug_mfile_ {
    aug_mpool* mpool_;
    aug_fd fd_;
    int flags_;
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

AUG_EXTERNC aug_result
aug_closemfile_(aug_mfile_t mfile)
{
    aug_mpool* mpool = mfile->mpool_;
    aug_result result;
    assert(mfile);

    if (mfile->mmap_) {

        aug_destroymmap(mfile->mmap_);

        if (mfile->resvd_ > mfile->size_) {

            if (AUG_ISFAIL(result = aug_ftruncate(mfile->fd_,
                                                  (off_t)mfile->size_))) {
                aug_fclose(mfile->fd_);
                goto last;
            }
        }
    }

    result = aug_fclose(mfile->fd_);

 last:
    aug_freemem(mpool, mfile);
    aug_release(mpool);
    return result;
}

AUG_EXTERNC aug_mfile_t
aug_openmfile_(aug_mpool* mpool, const char* path, int flags, mode_t mode,
               unsigned tail)
{
    aug_fd fd;
    size_t size;
    aug_mfile_t mfile;
    assert(path);

    /* Opening a file with the append flag set (where that file is to be
       memory mapped), does not seem to make much sense.  Therefore, the
       append flag if set is stripped off before the remaining flags are
       passed to the open file function. */

    if (AUG_BADFD == (fd = aug_fopen(path, flags & ~AUG_APPEND, mode)))
        return NULL;

    if (AUG_ISFAIL(aug_fsize(fd, &size)))
        return NULL;

    if (!(mfile = aug_allocmem(mpool, sizeof(struct aug_mfile_) + tail)))
        goto fail;

    mfile->mpool_ = mpool;
    mfile->fd_ = fd;
    mfile->flags_ = toflags_(flags);
    mfile->resvd_ = mfile->size_ = (unsigned)size;
    mfile->mmap_ = NULL;
    aug_retain(mpool);
    return mfile;

 fail:
    aug_fclose(fd);
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

            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",
                           AUG_EPERM, AUG_MSG("file is not writable"));
            return NULL;
        }

        resvd = reserve_(size);
        if (AUG_ISFAIL(aug_ftruncate(mfile->fd_, resvd - mfile->size_)))
            return NULL;

        mfile->resvd_ = resvd;
    }

    if (mfile->mmap_) {

        if (AUG_ISFAIL(aug_remmap(mfile->mmap_, 0, size))) {

            aug_destroymmap(mfile->mmap_);
            mfile->mmap_ = NULL;
            return NULL;
        }
    } else {

        mfile->mmap_ = aug_createmmap(mfile->mpool_, mfile->fd_, 0, size,
                                      mfile->flags_);
        if (!mfile->mmap_)
            return NULL;
    }

 done:
    if (size > mfile->size_)
        mfile->size_ = size;

    return mfile->mmap_->addr_;
}

AUG_EXTERNC aug_result
aug_syncmfile_(aug_mfile_t mfile)
{
    assert(mfile);
    if (mfile->mmap_)
        return aug_syncmmap(mfile->mmap_);

    return aug_fsync(mfile->fd_);
}

AUG_EXTERNC aug_result
aug_truncatemfile_(aug_mfile_t mfile, unsigned size)
{
    assert(mfile);
    if (!(mfile->flags_ & AUG_MMAPWR)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPERM,
                       AUG_MSG("file is not writable"));
        return AUG_FAILERROR;
    }
    mfile->size_ = size;
    return AUG_SUCCESS;
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

AUG_EXTERNC aug_mpool*
aug_mfilempool_(aug_mfile_t mfile)
{
    assert(mfile);
    aug_retain(mfile->mpool_);
    return mfile->mpool_;
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

