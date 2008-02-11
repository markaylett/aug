/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/file.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/errinfo.h"
#include "augctx/utility.h"

#include <assert.h>
#include <string.h>

#include <windows.h>
#include <fcntl.h>
#include <io.h>

static DWORD*
access_(DWORD* dst, int src)
{
    DWORD access;
    switch(src & (_O_RDONLY | _O_WRONLY | _O_RDWR)) {
    case _O_RDONLY:
        access = GENERIC_READ;
        break;
    case _O_WRONLY:
        access = GENERIC_WRITE;
        break;
    case _O_RDWR:
        access = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        return NULL;
    }
    if (src & _O_APPEND)
        access |= FILE_APPEND_DATA;

    *dst = access;
    return dst;
}

static DWORD
create_(int flags)
{
    DWORD create;
    switch (flags & (_O_CREAT | _O_EXCL | _O_TRUNC)) {
    case _O_CREAT | _O_TRUNC:
        create = CREATE_ALWAYS;
        break;
    case _O_CREAT | _O_EXCL:
    case _O_CREAT | _O_TRUNC | _O_EXCL:
        /* _O_TRUNC is meaningless with _O_CREAT. */
        create = CREATE_NEW;
        break;
    case _O_CREAT:
        create = OPEN_ALWAYS;
        break;
    case 0:
    case _O_EXCL:
        /* _O_EXCL is meaningless without _O_CREAT. */
        create = OPEN_EXISTING;
        break;
    case _O_TRUNC:
    case _O_TRUNC | _O_EXCL:
        /* _O_EXCL is meaningless without _O_CREAT. */
        create = TRUNCATE_EXISTING;
        break;
    default:
        /* Cannot fail. */
        break;
    }
    return create;
}

static int
doclose_(aug_ctx* ctx, aug_fd fd)
{
    if (!CloseHandle(fd)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return -1;
    }
    return 0;
}

static aug_fd
vopen_(aug_ctx* ctx, const char* path, int flags, va_list args)
{
    HANDLE h;
    SECURITY_ATTRIBUTES sa;
    DWORD access, attr;

    if (!access_(&access, flags)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            ERROR_NOT_SUPPORTED);
        return INVALID_HANDLE_VALUE;
    }

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (flags & _O_CREAT) {
        mode_t mode = va_arg(args, int);
        /* Read-only if no write bits set. */
        attr = (0 == (mode & 0222))
            ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL;
    } else
        attr = FILE_ATTRIBUTE_NORMAL;

    if (INVALID_HANDLE_VALUE
        == (h = CreateFile(path, access, FILE_SHARE_DELETE | FILE_SHARE_READ
                           | FILE_SHARE_WRITE, &sa, create_(flags), attr,
                           NULL))) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return INVALID_HANDLE_VALUE;
    }
    return h;
}

struct impl_ {
    aug_file file_;
    int refs_;
    aug_ctx* ctx_;
    aug_fd fd_;
};

static void*
cast_(aug_file* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_fileid)) {
        aug_retain(obj);
        return obj;
    } else if (AUG_EQUALID(id, aug_ctxid)) {
        struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
        aug_retain(impl->ctx_);
        return impl->ctx_;
    }
    return NULL;
}

static void
retain_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_ctx* ctx = impl->ctx_;
        aug_mpool* mpool = aug_getmpool(ctx);
        if (AUG_BADFD == impl->fd_)
            doclose_(ctx, impl->fd_);
        aug_free(mpool, impl);
        aug_release(mpool);
        aug_release(ctx);
    }
}

static int
close_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    int ret = doclose_(impl->ctx_, impl->fd_);
    impl->fd_ = AUG_BADFD;
    return ret;
}

static int
setnonblock_(aug_file* obj, int on)
{
    return 0;
}

static aug_fd
getfd_(aug_file* obj)
{
    return 0;
}

static const struct aug_filevtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    close_,
    setnonblock_,
    getfd_
};

static aug_file*
vcreatefile_(aug_ctx* ctx, const char* path, int flags, va_list args)
{
    struct impl_* impl;
    aug_mpool* mpool;
    aug_fd fd;
    assert(ctx);

    if (AUG_BADFD == (fd = vopen_(ctx, path, flags, args)))
        return NULL;

    mpool = aug_getmpool(ctx);
    impl = aug_malloc(mpool, sizeof(struct impl_));
    aug_release(mpool);

    if (!impl) {
        doclose_(ctx, fd);
        return NULL;
    }

    impl->file_.vtbl_ = &vtbl_;
    impl->file_.impl_ = NULL;
    impl->refs_ = 1;

    aug_retain(ctx);

    impl->ctx_ = ctx;
    impl->fd_ = fd;

    return &impl->file_;
}

AUGSYS_API aug_file*
aug_createfile(aug_ctx* ctx, const char* path, int flags, ...)
{
    aug_file* file;
    va_list args;
    va_start(args, flags);
    file = vcreatefile_(ctx, path, flags, args);
    va_end(args);
    return file;
}
