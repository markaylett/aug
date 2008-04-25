/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/utility.h" /* aug_filesize() */

#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/mman.h>

#define FLAGMASK_ (AUG_MMAPRD | AUG_MMAPWR)

typedef struct impl_ {
    struct aug_mmap mmap_;
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
    aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                   AUG_MSG("invalid protection flags '%d'"), (int)from);
    return -1;
}

static int
verify_(size_t size, size_t offset, size_t len)
{
    /* An empty file cannot be mapped, in addition, the size to be mapped
       should not be greater than the file size. */

    if (!size || size < (offset + len)) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid file map size '%d'"), (int)size);
        return -1;
    }

    /* The offset if specified must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
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
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
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
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUG_EXTERNC int
aug_destroymmap(struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    int ret = destroymmap_(impl);
    free(impl);
    return ret;
}

AUG_EXTERNC struct aug_mmap*
aug_createmmap(aug_fd fd, size_t offset, size_t len, int flags)
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

    if (!(impl = (impl_t)malloc(sizeof(struct impl_)))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    impl->mmap_.addr_ = NULL;
    impl->mmap_.len_ = 0;
    impl->fd_ = fd;
    impl->prot_ = prot;
    impl->size_ = size;

    if (-1 == createmmap_(impl, offset, len)) {
        free(impl);
        return NULL;
    }
    return (struct aug_mmap*)impl;
}

AUG_EXTERNC int
aug_remmap(struct aug_mmap* mm, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mm;
    void* addr = impl->mmap_.addr_;
    impl->mmap_.addr_ = NULL;

    if (addr && -1 == munmap(addr, impl->mmap_.len_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (impl->size_ < (offset + len)) {

        if (-1 == aug_filesize(impl->fd_, &impl->size_))
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
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
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
