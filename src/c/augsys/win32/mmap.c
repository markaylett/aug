/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/utility.h" /* aug_filesize() */
#include "augsys/windows.h"

#include <io.h>
#include <stdlib.h>         /* free() */

#define FLAGMASK_ (AUG_MMAPRD | AUG_MMAPWR)

typedef struct impl_ {
    struct aug_mmap_ mmap_;
    int fd_;
    DWORD prot_, access_;
    size_t size_;
    HANDLE mapping_;
}* impl_t;

static int
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
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                   AUG_MSG("invalid access flags '%d'"), from);
    return -1;
}

static int
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
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                   AUG_MSG("invalid protection flags '%d'"), from);
    return -1;
}

static int
verify_(size_t size, size_t offset, size_t len)
{
    /* An empty file cannot be mapped, in addition, the size to be mapped
       should not be greater than the file size. */

    if (!size || size < (offset + len)) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid file map size '%d'"), size);
        return -1;
    }

    /* The offset, if specified, must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid file map offset '%d'"), offset);
        return -1;
    }

    return 0;
}

static int
createmmap_(impl_t impl, size_t offset, size_t len)
{
    void* addr;
    HANDLE mapping;
    DWORD err;
    intptr_t file = _get_osfhandle(impl->fd_);

    if (-1 == file) {
        aug_setposixerrinfo(__FILE__, __LINE__, EBADF);
        return -1;
    }

    if (!len)
        len = impl->size_ - offset;

    mapping = impl->mapping_;

    if (INVALID_HANDLE_VALUE == mapping) {

        if (!(mapping = CreateFileMapping((HANDLE)file, NULL, impl->prot_, 0,
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
    aug_setwin32errinfo(__FILE__, __LINE__, err);
    return -1;
}

static int
freemmap_(impl_t impl)
{
    DWORD err = 0;

    if (impl->mmap_.addr_ && !UnmapViewOfFile(impl->mmap_.addr_))
        err = GetLastError();

    if (INVALID_HANDLE_VALUE != impl->mapping_ && !CloseHandle(impl->mapping_)
        && !err)
        err = GetLastError();

    if (err) {
        aug_setwin32errinfo(__FILE__, __LINE__, err);
        return -1;
    }
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
    DWORD prot, access;
    size_t size;

    if (-1 == toprot_(&prot, flags))
        return NULL;

    if (-1 == toaccess_(&access, flags))
        return NULL;

    if (-1 == aug_filesize(fd, &size))
        return NULL;

    if (-1 == verify_(size, offset, len))
        return NULL;

    if (!(impl = (impl_t)malloc(sizeof(struct impl_)))) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    impl->mmap_.addr_ = NULL;
    impl->mmap_.len_ = 0;
    impl->fd_ = fd;
    impl->prot_ = prot;
    impl->access_ = access;
    impl->size_ = size;
    impl->mapping_ = INVALID_HANDLE_VALUE;

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

    if (addr && !UnmapViewOfFile(addr)) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (impl->size_ < (offset + len)) {

        HANDLE mapping = impl->mapping_;
        impl->mapping_ = INVALID_HANDLE_VALUE;

        if (INVALID_HANDLE_VALUE != mapping && !CloseHandle(mapping)) {
            aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
            return -1;
        }

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

    if (!FlushViewOfFile(impl->mmap_.addr_, impl->mmap_.len_)) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
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
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwAllocationGranularity;
}

AUGSYS_EXTERN size_t
aug_pagesize(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}
