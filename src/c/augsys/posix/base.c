/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/lock.h"
#include "augsys/log.h"

AUGSYS_API int
aug_init(void)
{
    return aug_initlock_();
}

AUGSYS_API int
aug_term(void)
{
    aug_setlogger(NULL);
    return aug_termlock_();
}
