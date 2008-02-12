#include "augctx/errinfo.h"

#include "augsys/windows.h"

#include <fcntl.h>

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

AUGSYS_API aug_status
aug_close(aug_ctx* ctx, aug_fd fd)
{
    if (!CloseHandle(fd)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_status
aug_fsync(aug_ctx* ctx, aug_fd fd)
{
    if (!FlushFileBuffers(fd)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_status
aug_ftruncate(aug_ctx* ctx, aug_fd fd, off_t size)
{
    aug_status ret;
    LARGE_INTEGER li, orig;

    /* Store current position for later restoration. */

    li.QuadPart = 0;
    if (!SetFilePointerEx(fd, li, &orig, FILE_CURRENT)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return AUG_FAILURE;
    }

    /* Move pointer to required size. */

    li.QuadPart = (LONGLONG)size;
    if (!SetFilePointerEx(fd, li, NULL, FILE_BEGIN)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return AUG_FAILURE;
    }

    /* Truncate.  Note: this will not fill the gap with zeros. */

    if (!SetEndOfFile(fd)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        ret = AUG_FAILURE;
    } else
        ret = AUG_SUCCESS;

    /* Restore original position. */

    SetFilePointerEx(fd, orig, NULL, FILE_BEGIN);
    return ret;
}

AUGSYS_API aug_fd
aug_vopen(aug_ctx* ctx, const char* path, int flags, va_list args)
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

AUGSYS_API aug_fd
aug_open(aug_ctx* ctx, const char* path, int flags, ...)
{
    aug_fd fd;
    va_list args;
    va_start(args, flags);
    fd = aug_vopen(ctx, path, flags, args);
    va_end(args);
    return fd;
}

AUGSYS_API aug_status
aug_pipe(aug_ctx* ctx, aug_fd fds[2])
{
    return AUG_SUCCESS;
}

AUGSYS_API ssize_t
aug_read(aug_ctx* ctx, aug_fd fd, void* buf, size_t size)
{
    return 0;
}

AUGSYS_API ssize_t
aug_write(aug_ctx* ctx, aug_fd fd, const void* buf, size_t size)
{
    return 0;
}

AUGSYS_API void
aug_msleep(aug_ctx* ctx, unsigned ms)
{
    Sleep(ms);
}
