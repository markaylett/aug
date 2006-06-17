/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errno.h"
#include "augsys/lock.h"
#include "augsys/log.h"

#include <winsock2.h>

AUGSYS_API int
aug_init(void)
{
    WSADATA data;
    int err;

    if (-1 == aug_initlock_())
        return -1;

    if (0 != (err = WSAStartup(MAKEWORD(2, 2), &data))) {
        aug_maperror(err);
        aug_termlock_();
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_term(void)
{
    aug_setlogger(NULL);

    if (SOCKET_ERROR == WSACleanup()) {
        aug_maperror(WSAGetLastError());
        aug_termlock_();
        return -1;
    }

    return aug_termlock_();
}
