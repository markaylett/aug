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
#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augsys/windows.h"

#include <fcntl.h>

static DWORD*
access_(int src, DWORD* dst)
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
    DWORD create = 0;
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
aug_fclose_I(aug_fd fd)
{
    if (!CloseHandle(fd)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_fsetnonblock_AI(aug_fd fd, aug_bool on)
{
    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                    AUG_MSG("aug_fsetnonblock() not supported"));
    return -1;
}

AUGSYS_API aug_fd
aug_vfopen_N(const char* path, int flags, va_list args)
{
    DWORD access, attr;
    SECURITY_ATTRIBUTES sa;
    HANDLE h;

    if (!access_(flags, &access)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, ERROR_NOT_SUPPORTED);
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
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
    }
    return h;
}

AUGSYS_API aug_fd
aug_fopen_N(const char* path, int flags, ...)
{
    aug_fd fd;
    va_list args;
    va_start(args, flags);
    fd = aug_vfopen_N(path, flags, args);
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

    if (!CreatePipe(&rd, &wr, &sa, 0)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    fds[0] = rd;
    fds[1] = wr;

    return 0;
}

AUGSYS_API aug_rsize
aug_fread_AI(aug_fd fd, void* buf, size_t size)
{
    DWORD ret;
    if (!ReadFile(fd, buf, (DWORD)size, &ret, NULL)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return (ssize_t)ret;
}

AUGSYS_API aug_rsize
aug_fwrite_AI(aug_fd fd, const void* buf, size_t size)
{
    DWORD ret;
    if (!WriteFile(fd, buf, (DWORD)size, &ret, NULL)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return (ssize_t)ret;
}

AUGSYS_API aug_result
aug_fsync(aug_fd fd)
{
    if (!FlushFileBuffers(fd)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_ftruncate_AI(aug_fd fd, off_t size)
{
    LARGE_INTEGER li, orig;
    aug_result result;

    /* Store current position for later restoration. */

    li.QuadPart = 0;
    if (!SetFilePointerEx(fd, li, &orig, FILE_CURRENT)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    /* Move pointer to required size. */

    li.QuadPart = (LONGLONG)size;
    if (!SetFilePointerEx(fd, li, NULL, FILE_BEGIN)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    /* Truncate.  Note: this will not fill the gap with zeros. */

    if (SetEndOfFile(fd))
        result = 0;
    else {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        result = -1;
    }

    /* Restore original position. */

    SetFilePointerEx(fd, orig, NULL, FILE_BEGIN);
    return result;
}

AUGSYS_API aug_result
aug_fsize_IN(aug_fd fd, size_t* size)
{
    LARGE_INTEGER li;

    if (!GetFileSizeEx(fd, &li)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    *size = (size_t)li.QuadPart;
    return 0;
}

AUGSYS_API void
aug_msleep_I(unsigned ms)
{
    Sleep(ms);
}
