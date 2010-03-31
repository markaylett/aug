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
#include "augsys/unistd.h" /* aug_fsize_IN() */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <io.h>

#define FLAGMASK_ (AUG_MMAPRD | AUG_MMAPWR)

typedef struct impl_ {
    struct aug_mmap mmap_;
    aug_mpool* mpool_;
    aug_fd fd_;
    DWORD prot_, access_;
    size_t size_;
    HANDLE mapping_;
}* impl_t;

static aug_result
toaccess_(DWORD* to, int from)
{
    DWORD access = 0;
    if (from & ~FLAGMASK_)
        goto fail;

    if (AUG_MMAPRD == (from & AUG_MMAPRD))
        access |= FILE_MAP_READ;

    if (AUG_MMAPWR == (from & AUG_MMAPWR))
        access |= FILE_MAP_WRITE;

    *to = access;
    return 0;

 fail:
    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                    AUG_MSG("invalid access flags [%d]"), (int)from);
    return -1;
}

static aug_result
toprot_(DWORD* to, int from)
{
    DWORD prot = 0;
    if (from & ~FLAGMASK_)
        goto fail;

    if (AUG_MMAPRD != (from & AUG_MMAPRD))
        goto fail;

    if (AUG_MMAPWR == (from & AUG_MMAPWR))
        prot = PAGE_READWRITE;
    else
        prot = PAGE_READONLY;

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

    /* The offset, if specified, must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("invalid file map offset [%d]"), (int)offset);
        return -1;
    }

    return 0;
}

static aug_result
createmmap_(impl_t impl, size_t offset, size_t len)
{
    void* addr;
    HANDLE mapping;
    DWORD err;

    if (AUG_BADFD == impl->fd_) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, EBADF);
        return -1;
    }

    if (!len)
        len = impl->size_ - offset;

    mapping = impl->mapping_;

    if (INVALID_HANDLE_VALUE == mapping) {

        if (!(mapping = CreateFileMapping(impl->fd_, NULL, impl->prot_, 0,
                                          (DWORD)impl->size_, NULL))) {
            err = GetLastError();
            goto fail1;
        }
    }

    if (!(addr = MapViewOfFile(mapping, impl->access_, 0,
                               (DWORD)offset, len))) {
        err = GetLastError();
        goto fail2;
    }

    impl->mmap_.addr_ = addr;
    impl->mmap_.len_ = len;
    impl->mapping_ = mapping;

    return 0;

 fail2:
    if (INVALID_HANDLE_VALUE == impl->mapping_)
        CloseHandle(mapping);
 fail1:
    aug_setwin32error(aug_tlx, __FILE__, __LINE__, err);
    return -1;
}

static aug_result
destroymmap_(impl_t impl)
{
    DWORD err = 0;

    if (impl->mmap_.addr_ && !UnmapViewOfFile(impl->mmap_.addr_))
        err = GetLastError();

    if (INVALID_HANDLE_VALUE != impl->mapping_ && !CloseHandle(impl->mapping_)
        && !err)
        err = GetLastError();

    if (err) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, err);
        return -1;
    }

    return 0;
}

AUGSYS_API void
aug_destroymmap_A(struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    aug_mpool* mpool = impl->mpool_;
    destroymmap_(impl);
    aug_freemem(mpool, impl);
    aug_release(mpool);
}

AUGSYS_API struct aug_mmap*
aug_createmmap_AIN(aug_mpool* mpool, aug_fd fd, size_t offset, size_t len,
                   int flags)
{
    impl_t impl;
    DWORD prot, access;
    size_t size;

    if (toprot_(&prot, flags) < 0)
        return NULL;

    if (toaccess_(&access, flags) < 0)
        return NULL;

    if (aug_fsize_IN(fd, &size) < 0)
        return NULL;

    if (verify_(size, offset, len) < 0)
        return NULL;

    if (!(impl = aug_allocmem(mpool, sizeof(struct impl_))))
        return NULL;

    impl->mmap_.addr_ = NULL;
    impl->mmap_.len_ = 0;
    impl->mpool_ = mpool;
    impl->fd_ = fd;
    impl->prot_ = prot;
    impl->access_ = access;
    impl->size_ = size;
    impl->mapping_ = INVALID_HANDLE_VALUE;

    if (createmmap_(impl, offset, len) < 0) {
        aug_freemem(mpool, impl);
        return NULL;
    }
    aug_retain(mpool);
    return (struct aug_mmap*)impl;
}

AUGSYS_API aug_result
aug_remmap_AIN(struct aug_mmap* mm, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mm;
    void* addr = impl->mmap_.addr_;

    impl->mmap_.addr_ = NULL;

    if (addr && !UnmapViewOfFile(addr)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (impl->size_ < (offset + len)) {

        HANDLE mapping = impl->mapping_;
        impl->mapping_ = INVALID_HANDLE_VALUE;

        if (INVALID_HANDLE_VALUE != mapping && !CloseHandle(mapping)) {
            aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
            return -1;
        }

        if (aug_fsize_IN(impl->fd_, &impl->size_) < 0)
            return -1;
    }

    if (verify_(impl->size_, offset, len) < 0)
        return -1;

    return createmmap_(impl, offset, len);
}

AUGSYS_API aug_result
aug_syncmmap(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;

    if (!FlushViewOfFile(impl->mmap_.addr_, impl->mmap_.len_)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
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
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwAllocationGranularity;
}

AUGSYS_API unsigned
aug_pagesize(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}
