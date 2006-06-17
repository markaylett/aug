/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <signal.h>

AUGSRV_API int
aug_sigactions(void (*handler)(int))
{
    signal(SIGINT, handler);
    signal(SIGTERM, handler);
    return 0;
}

AUGSRV_API int
aug_sigblock(void)
{
    return 0;
}

AUGSRV_API int
aug_sigunblock(void)
{
    return 0;
}
