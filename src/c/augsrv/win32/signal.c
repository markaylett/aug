/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <signal.h>

AUGSRV_API int
aug_signalhandler(void (*handler)(int))
{
    signal(SIGINT, handler);
    signal(SIGTERM, handler);
    return 0;
}

AUGSRV_API int
aug_blocksignals(void)
{
    return 0;
}

AUGSRV_API int
aug_unblocksignals(void)
{
    return 0;
}
