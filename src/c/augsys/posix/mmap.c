/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/unistd.h" /* aug_fsize() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/mman.h>

#define FLAGMASK_ (AUG_MMAPRD | AUG_MMAPWR)

typedef struct impl_ {
    struct aug_mmap mmap_;
    aug_mpool* mpool_;
    aug_fd fd_, prot_;
    size_t size_;
}* impl_t;

static int
toprot_(int* to, int from)
{
    int prot = 0;
    if (from & ~FLAGMASK_)
        goto fail;

    if (AUG_MMAPRD == (from & AUG_MMAPRD))
        prot |= PROT_READ;

    if (AUG_MMAPWR == (from & AUG_MMAPWR))
        prot |= PROT_WRITE;

    if (!prot)
        prot = PROT_NONE;

    *to = prot;
    return 0;

 fail:
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid protection flags '%d'"), (int)from);
    return -1;
}

static int
verify_(size_t size, size_t offset, size_t len)
{
    /* An empty file cannot be mapped, in addition, the size to be mapped
       should not be greater than the file size. */

    if (!size || size < (offset + len)) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid file map size '%d'"), (int)size);
        return -1;
    }

    /* The offset if specified must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid file map offset '%d'"), (int)offset);
        return -1;
    }

    return 0;
}

static int
createmmap_(impl_t impl, size_t offset, size_t len)
{
    void* addr;

    if (!len)
        len = impl->size_ - offset;

    if (MAP_FAILED == (addr = mmap(NULL, len, impl->prot_, MAP_SHARED,
                                   impl->fd_, (off_t)offset))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }

    impl->mmap_.addr_ = addr;
    impl->mmap_.len_ = len;
    return 0;
}

static int
destroymmap_(impl_t impl)
{
    if (impl->mmap_.addr_
        && -1 == munmap(impl->mmap_.addr_, impl->mmap_.len_)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUG_EXTERNC void
aug_destroymmap(struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    aug_mpool* mpool = impl->mpool_;
    destroymmap_(impl);
    aug_freemem(mpool, impl);
    aug_release(mpool);
}

AUG_EXTERNC struct aug_mmap*
aug_createmmap(aug_mpool* mpool, aug_fd fd, size_t offset, size_t len,
               int flags)
{
    impl_t impl;
    int prot;
    size_t size;

    if (-1 == toprot_(&prot, flags))
        return NULL;

    if (-1 == aug_fsize(fd, &size))
        return NULL;

    if (-1 == verify_(size, offset, len))
        return NULL;

    if (!(impl = aug_allocmem(mpool, sizeof(struct impl_))))
        return NULL;

    impl->mmap_.addr_ = NULL;
    impl->mmap_.len_ = 0;
    impl->mpool_ = mpool;
    impl->fd_ = fd;
    impl->prot_ = prot;
    impl->size_ = size;

    if (-1 == createmmap_(impl, offset, len)) {
        aug_freemem(mpool, impl);
        return NULL;
    }
    aug_retain(mpool);
    return (struct aug_mmap*)impl;
}

AUG_EXTERNC int
aug_remmap(struct aug_mmap* mm, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mm;
    void* addr = impl->mmap_.addr_;
    impl->mmap_.addr_ = NULL;

    if (addr && -1 == munmap(addr, impl->mmap_.len_)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }

    if (impl->size_ < (offset + len)) {

        if (-1 == aug_fsize(impl->fd_, &impl->size_))
            return -1;
    }

    if (-1 == verify_(impl->size_, offset, len))
        return -1;

    return createmmap_(impl, offset, len);
}

AUG_EXTERNC int
aug_syncmmap(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    if (-1 == msync(impl->mmap_.addr_, impl->mmap_.len_, MS_SYNC)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUG_EXTERNC size_t
aug_mmapsize(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    return impl->size_;
}

AUG_EXTERNC unsigned
aug_granularity(void)
{
    /* Not documented to return an error. */

    return getpagesize();
}

AUG_EXTERNC unsigned
aug_pagesize(void)
{
    /* Not documented to return an error. */

    return getpagesize();
}
