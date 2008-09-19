#include "augctx/base.h"
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

AUGSYS_API aug_result
aug_fclose(aug_fd fd)
{
    if (!CloseHandle(fd))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());
    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_fsetnonblock(aug_fd fd, aug_bool on)
{
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                   AUG_MSG("aug_fsetnonblock() not supported"));
    return AUG_FAILERROR;
}

AUGSYS_API aug_fd
aug_vfopen(const char* path, int flags, va_list args)
{
    DWORD access, attr;
    SECURITY_ATTRIBUTES sa;
    HANDLE h;

    if (!access_(&access, flags)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                            ERROR_NOT_SUPPORTED);
        return INVALID_HANDLE_VALUE;
    }

    if (flags & _O_CREAT) {
        mode_t mode = va_arg(args, int);
        /* Read-only if no write bits set. */
        attr = (0 == (mode & 0222))
            ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL;
    } else
        attr = FILE_ATTRIBUTE_NORMAL;

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (INVALID_HANDLE_VALUE
        == (h = CreateFile(path, access, FILE_SHARE_DELETE | FILE_SHARE_READ
                           | FILE_SHARE_WRITE, &sa, create_(flags), attr,
                           NULL))) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
    }
    return h;
}

AUGSYS_API aug_fd
aug_fopen(const char* path, int flags, ...)
{
    aug_fd fd;
    va_list args;
    va_start(args, flags);
    fd = aug_vfopen(path, flags, args);
    va_end(args);
    return fd;
}

AUGSYS_API aug_result
aug_fpipe(aug_fd fds[2])
{
    aug_fd rd, wr;
    SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&rd, &wr, &sa, 0))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    fds[0] = rd;
    fds[1] = wr;

    return AUG_SUCCESS;
}

AUGSYS_API aug_rsize
aug_fread(aug_fd fd, void* buf, size_t size)
{
    DWORD ret;
    if (!ReadFile(fd, buf, (DWORD)size, &ret, NULL))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    return AUG_MKRESULT((ssize_t)ret);
}

AUGSYS_API aug_rsize
aug_fwrite(aug_fd fd, const void* buf, size_t size)
{
    DWORD ret;
    if (!WriteFile(fd, buf, (DWORD)size, &ret, NULL))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    return AUG_MKRESULT((ssize_t)ret);
}

AUGSYS_API aug_result
aug_fsync(aug_fd fd)
{
    if (!FlushFileBuffers(fd))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_ftruncate(aug_fd fd, off_t size)
{
    LARGE_INTEGER li, orig;
    aug_result result;

    /* Store current position for later restoration. */

    li.QuadPart = 0;
    if (!SetFilePointerEx(fd, li, &orig, FILE_CURRENT))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    /* Move pointer to required size. */

    li.QuadPart = (LONGLONG)size;
    if (!SetFilePointerEx(fd, li, NULL, FILE_BEGIN))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    /* Truncate.  Note: this will not fill the gap with zeros. */

    if (SetEndOfFile(fd))
        result = AUG_SUCCESS;
    else
        result = aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                     GetLastError());

    /* Restore original position. */

    SetFilePointerEx(fd, orig, NULL, FILE_BEGIN);
    return result;
}

AUGSYS_API aug_result
aug_fsize(aug_fd fd, size_t* size)
{
    LARGE_INTEGER li;

    if (!GetFileSizeEx(fd, &li))
        return aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__,
                                   GetLastError());

    *size = (size_t)li.QuadPart;
    return AUG_SUCCESS;
}

AUGSYS_API void
aug_msleep(unsigned ms)
{
    Sleep(ms);
}
