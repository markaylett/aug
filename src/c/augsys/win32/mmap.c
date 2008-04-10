/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errno.h"
#include "augsys/unistd.h" /* aug_fsize() */
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <io.h>
#include <stdlib.h>         /* free() */

#define FLAGMASK_ (AUG_MMAPRD | AUG_MMAPWR)

typedef struct impl_ {
    struct aug_mmap mmap_;
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
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid access flags '%d'"), (int)from);
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

    /* The offset, if specified, must occur on an allocation size boundary. */

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
    HANDLE mapping;
    DWORD err;
    intptr_t file;

    if (-1 == impl->fd_) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EBADF);
        return -1;
    }

    file = _get_osfhandle(impl->fd_);

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
    aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, err);
    return -1;
}

static int
destroymmap_(impl_t impl)
{
    DWORD err = 0;

    if (impl->mmap_.addr_ && !UnmapViewOfFile(impl->mmap_.addr_))
        err = GetLastError();

    if (INVALID_HANDLE_VALUE != impl->mapping_ && !CloseHandle(impl->mapping_)
        && !err)
        err = GetLastError();

    if (err) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, err);
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
aug_createmmap(int fd, size_t offset, size_t len, int flags)
{
    impl_t impl;
    DWORD prot, access;
    size_t size;

    if (-1 == toprot_(&prot, flags))
        return NULL;

    if (-1 == toaccess_(&access, flags))
        return NULL;

    if (AUG_FAILURE == aug_fsize((aug_fd)_get_osfhandle(fd), &size))
        return NULL;

    if (-1 == verify_(size, offset, len))
        return NULL;

    if (!(impl = (impl_t)malloc(sizeof(struct impl_)))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);
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
    return (struct aug_mmap*)impl;
}

AUG_EXTERNC int
aug_remmap(struct aug_mmap* mm, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mm;
    void* addr = impl->mmap_.addr_;
    impl->mmap_.addr_ = NULL;

    if (addr && !UnmapViewOfFile(addr)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    if (impl->size_ < (offset + len)) {

        HANDLE mapping = impl->mapping_;
        impl->mapping_ = INVALID_HANDLE_VALUE;

        if (INVALID_HANDLE_VALUE != mapping && !CloseHandle(mapping)) {
            aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                GetLastError());
            return -1;
        }

        if (-1 == aug_fsize((aug_fd)_get_osfhandle(impl->fd_), &impl->size_))
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

    if (!FlushViewOfFile(impl->mmap_.addr_, impl->mmap_.len_)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
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
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwAllocationGranularity;
}

AUG_EXTERNC unsigned
aug_pagesize(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}
