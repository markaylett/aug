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
    return AUG_SUCCESS;

 fail:
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid access flags [%d]"), (int)from);
    return AUG_FAILERROR;
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
    return AUG_SUCCESS;

 fail:
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid protection flags [%d]"), (int)from);
    return AUG_FAILERROR;
}

static aug_result
verify_(size_t size, size_t offset, size_t len)
{
    /* An empty file cannot be mapped, in addition, the size to be mapped
       should not be greater than the file size. */

    if (!size || size < (offset + len)) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid file map size [%d]"), (int)size);
        return AUG_FAILERROR;
    }

    /* The offset, if specified, must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid file map offset [%d]"), (int)offset);
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

static aug_result
createmmap_(impl_t impl, size_t offset, size_t len)
{
    void* addr;
    HANDLE mapping;
    DWORD err;

    if (AUG_BADFD == impl->fd_)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EBADF);

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

    return AUG_SUCCESS;

 fail2:
    if (INVALID_HANDLE_VALUE == impl->mapping_)
        CloseHandle(mapping);
 fail1:
    return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, err);
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

    if (err)
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, err);

    return AUG_SUCCESS;
}

AUGSYS_API void
aug_destroymmap(struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    aug_mpool* mpool = impl->mpool_;
    destroymmap_(impl);
    aug_freemem(mpool, impl);
    aug_release(mpool);
}

AUGSYS_API struct aug_mmap*
aug_createmmap(aug_mpool* mpool, aug_fd fd, size_t offset, size_t len,
               int flags)
{
    impl_t impl;
    DWORD prot, access;
    size_t size;

    if (aug_isfail(toprot_(&prot, flags)))
        return NULL;

    if (aug_isfail(toaccess_(&access, flags)))
        return NULL;

    if (aug_isfail(aug_fsize(fd, &size)))
        return NULL;

    if (aug_isfail(verify_(size, offset, len)))
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

    if (aug_isfail(createmmap_(impl, offset, len))) {
        aug_freemem(mpool, impl);
        return NULL;
    }
    aug_retain(mpool);
    return (struct aug_mmap*)impl;
}

AUGSYS_API aug_result
aug_remmap(struct aug_mmap* mm, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mm;
    void* addr = impl->mmap_.addr_;

    impl->mmap_.addr_ = NULL;

    if (addr && !UnmapViewOfFile(addr))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    if (impl->size_ < (offset + len)) {

        HANDLE mapping = impl->mapping_;
        impl->mapping_ = INVALID_HANDLE_VALUE;

        if (INVALID_HANDLE_VALUE != mapping && !CloseHandle(mapping))
            return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                       GetLastError());

        aug_verify(aug_fsize(impl->fd_, &impl->size_));
    }

    aug_verify(verify_(impl->size_, offset, len));
    return createmmap_(impl, offset, len);
}

AUGSYS_API aug_result
aug_syncmmap(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;

    if (!FlushViewOfFile(impl->mmap_.addr_, impl->mmap_.len_))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());
    return AUG_SUCCESS;
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
