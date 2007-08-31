/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/lock.h"
#include "augsys/log.h"

#include <io.h>
#include <time.h> /* tzset() */
#include <winsock2.h>

static long timezone_ = 0;

static int
settimezone_(void)
{
	TIME_ZONE_INFORMATION tz;
    switch (GetTimeZoneInformation(&tz)) {
    case TIME_ZONE_ID_INVALID:
        aug_setwin32errno(GetLastError());
        return -1;
    case TIME_ZONE_ID_UNKNOWN:
        aug_seterrno(EINVAL);
        return -1;
    case TIME_ZONE_ID_STANDARD:
    case TIME_ZONE_ID_DAYLIGHT:
        break;
    }
    timezone_ = (tz.Bias + tz.StandardBias) * 60;
    return 0;
}

static int
close_(int fd)
{
    if (-1 == close(fd)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static ssize_t
read_(int fd, void* buf, size_t size)
{
    DWORD ret;

    if (!ReadFile((HANDLE)_get_osfhandle(fd), buf, (DWORD)size, &ret, NULL)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    DWORD ret;

    if (!WriteFile((HANDLE)_get_osfhandle(fd), buf, (DWORD)len, &ret,
                   NULL)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static struct aug_fdtype fdtype_ = {
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
    int err, ret = retain_();

    if (PROCEED_ != ret)
        return ret;

    tzset();
    if (-1 == settimezone_() || -1 == aug_initlock_())
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

    aug_initlog_();

    return 0;
}

AUGSYS_API int
aug_term(void)
{
    int ret = release_();
    if (PROCEED_ != ret)
        return ret;

    aug_termlog_();

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

AUGSYS_API long
aug_timezone(void)
{
    return timezone_;
}

AUGSYS_API const struct aug_fdtype*
aug_posixfdtype(void)
{
    return &fdtype_;
}

AUGSYS_API intptr_t
aug_getosfd(int fd)
{
    return (long)_get_osfhandle(fd);
}
