/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <signal.h>

AUGSRV_API aug_result
aug_setsighandler(void (*handler)(int))
{
    signal(SIGINT, handler);
    signal(SIGTERM, handler);
    return AUG_SUCCESS;
}

AUGSRV_API aug_result
aug_blocksignals(void)
{
    return AUG_SUCCESS;
}

AUGSRV_API aug_result
aug_unblocksignals(void)
{
    return AUG_SUCCESS;
}
