/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/utility.h" /* aug_filesize() */

#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/mman.h>

#define FLAGMASK_ (AUG_MMAPRD | AUG_MMAPWR)

typedef struct impl_ {
    struct aug_mmap_ mmap_;
    int fd_, prot_;
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
    errno = EINVAL;
    return -1;
}

static int
verify_(size_t size, size_t offset, size_t len)
{
    /* An empty file cannot be mapped, in addition, the size to be mapped
       should not be greater than the file size. */

    if (!size || size < (offset + len)) {
        errno = EINVAL;
        return -1;
    }

    /* The offset if specified must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        errno = EINVAL;
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
                                   impl->fd_, (off_t)offset)))
        return -1;

    impl->mmap_.addr_ = addr;
    impl->mmap_.len_ = len;
    return 0;
}

static int
freemmap_(impl_t impl)
{
    if (impl->mmap_.addr_)
        return munmap(impl->mmap_.addr_, impl->mmap_.len_);
    return 0;
}

AUGSYS_EXTERN int
aug_freemmap(struct aug_mmap_* mmap_)
{
    impl_t impl = (impl_t)mmap_;
    int ret = freemmap_(impl);
    free(impl);
    return ret;
}

AUGSYS_EXTERN struct aug_mmap_*
aug_createmmap(int fd, size_t offset, size_t len, int flags)
{
    impl_t impl;
    int prot;
    size_t size;

    if (-1 == toprot_(&prot, flags))
        return NULL;

    if (-1 == aug_filesize(fd, &size))
        return NULL;

    if (-1 == verify_(size, offset, len))
        return NULL;

    if (!(impl = (impl_t)malloc(sizeof(struct impl_))))
        return NULL;

    impl->mmap_.addr_ = NULL;
    impl->mmap_.len_ = 0;
    impl->fd_ = fd;
    impl->prot_ = prot;
    impl->size_ = size;

    if (-1 == createmmap_(impl, offset, len)) {
        free(impl);
        return NULL;
    }
    return (struct aug_mmap_*)impl;
}

AUGSYS_EXTERN int
aug_remmap(struct aug_mmap_* mmap_, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mmap_;
    void* addr = impl->mmap_.addr_;
    impl->mmap_.addr_ = NULL;

    if (addr && -1 == munmap(addr, impl->mmap_.len_))
        return -1;

    if (impl->size_ < (offset + len)) {

        if (-1 == aug_filesize(impl->fd_, &impl->size_))
            return -1;
    }

    if (-1 == verify_(impl->size_, offset, len))
        return -1;

    return createmmap_(impl, offset, len);
}

AUGSYS_EXTERN int
aug_syncmmap(struct aug_mmap_* mmap_)
{
    impl_t impl = (impl_t)mmap_;
    return msync(impl->mmap_.addr_, impl->mmap_.len_, MS_SYNC);
}

AUGSYS_EXTERN size_t
aug_mmapsize(struct aug_mmap_* mmap_)
{
    impl_t impl = (impl_t)mmap_;
    return impl->size_;
}

AUGSYS_EXTERN size_t
aug_granularity(void)
{
    return getpagesize();
}

AUGSYS_EXTERN size_t
aug_pagesize(void)
{
    return getpagesize();
}
