/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

static aug_result
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
    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                    AUG_MSG("invalid protection flags [%d]"), (int)from);
    return -1;
}

static aug_result
verify_(size_t size, size_t offset, size_t len)
{
    /* An empty file cannot be mapped, in addition, the size to be mapped
       should not be greater than the file size. */

    if (!size || size < (offset + len)) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid file map size [%d]"), (int)size);
        return -1;
    }

    /* The offset if specified must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid file map offset [%d]"), (int)offset);
        return -1;
    }

    return 0;
}

static aug_result
createmmap_A_(impl_t impl, size_t offset, size_t len)
{
    void* addr;

    if (!len)
        len = impl->size_ - offset;

    /* SYSCALL: mmap: EAGAIN */
    if (MAP_FAILED == (addr = mmap(NULL, len, impl->prot_, MAP_SHARED,
                                   impl->fd_, (off_t)offset))) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    impl->mmap_.addr_ = addr;
    impl->mmap_.len_ = len;
    return 0;
}

static aug_result
destroymmap_A_(impl_t impl)
{
    /* SYSCALL: munmap: EAGAIN */
    if (impl->mmap_.addr_
        && munmap(impl->mmap_.addr_, impl->mmap_.len_) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API void
aug_destroymmap_a(struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    aug_mpool* mpool = impl->mpool_;
    destroymmap_a_(impl);
    aug_freemem(mpool, impl);
    aug_release(mpool);
}

AUGSYS_API struct aug_mmap*
aug_createmmap_a(aug_mpool* mpool, aug_fd fd, size_t offset, size_t len,
                 int flags)
{
    impl_t impl;
    int prot;
    size_t size;

    if (toprot_(&prot, flags) < 0
        || aug_fsize(fd, &size) < 0
        || verify_(size, offset, len) < 0
        || !(impl = aug_allocmem(mpool, sizeof(struct impl_))))
        return NULL;

    impl->mmap_.addr_ = NULL;
    impl->mmap_.len_ = 0;
    impl->mpool_ = mpool;
    impl->fd_ = fd;
    impl->prot_ = prot;
    impl->size_ = size;

    if (createmmap_a_(impl, offset, len) < 0) {
        aug_freemem(mpool, impl);
        return NULL;
    }
    aug_retain(mpool);
    return (struct aug_mmap*)impl;
}

AUGSYS_API aug_result
aug_remmap_A(struct aug_mmap* mm, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mm;
    void* addr = impl->mmap_.addr_;

    impl->mmap_.addr_ = NULL;

    /* SYSCALL: munmap: EINTR */
    if (addr && munmap(addr, impl->mmap_.len_) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    if (impl->size_ < (offset + len)
        && aug_fsize(impl->fd_, &impl->size_) < 0)
        return -1;

    if (verify_(impl->size_, offset, len) < 0)
        return -1;

    return createmmap_A_(impl, offset, len);
}

AUGSYS_API aug_result
aug_syncmmap(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    if (msync(impl->mmap_.addr_, impl->mmap_.len_, MS_SYNC) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API size_t
aug_mmapsize(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    return impl->size_;
}

AUGSYS_API unsigned
aug_granularity(void)
{
    /* Not documented to return an error. */

    return getpagesize();
}

AUGSYS_API unsigned
aug_pagesize(void)
{
    /* Not documented to return an error. */

    return getpagesize();
}
