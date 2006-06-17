/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/lock.h"
#include "augsys/log.h"

#include <io.h>
#include <winsock2.h>

static int
close_(int fd)
{
    if (-1 == close(fd)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static ssize_t
read_(int fd, void* buf, size_t size)
{
    DWORD ret;

    if (!ReadFile((HANDLE)_get_osfhandle(fd), buf, (DWORD)size, &ret, NULL)) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static ssize_t
write_(int fd, const void* buf, size_t size)
{
    DWORD ret;

    if (!WriteFile((HANDLE)_get_osfhandle(fd), buf, (DWORD)size, &ret,
                   NULL)) {
        aug_setwin32errinfo(__FILE__, __LINE__, GetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static struct aug_fddriver driver_ = {
    close_,
    read_,
    NULL,
    write_,
    NULL,
    NULL
};

AUGSYS_API int
aug_init(struct aug_errinfo* errinfo)
{
    WSADATA data;
    int err;

    if (-1 == aug_initlock_())
        return -1;

    if (-1 == aug_initerrinfo_(errinfo)) {
        aug_termlock_();
        return -1;
    }

    if (0 != (err = WSAStartup(MAKEWORD(2, 2), &data))) {
        aug_setwin32errno(err);
        aug_termlock_();
        aug_termerrinfo_();
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_term(void)
{
    aug_setlogger(NULL);

    if (SOCKET_ERROR == WSACleanup()) {
        aug_setwin32errno(WSAGetLastError());
        aug_termerrinfo_();
        aug_termlock_();
        return -1;
    }

    if (-1 == aug_termerrinfo_()) {
        aug_termlock_();
        return -1;
    }

    return aug_termlock_();
}

AUGSYS_API const struct aug_fddriver*
aug_posixdriver(void)
{
    return &driver_;
}
