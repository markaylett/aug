/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_LOCK_H
#define AUGSYS_LOCK_H

#include "augsys/config.h"

#if defined(AUGSYS_BUILD)
AUGSYS_EXTERN int
aug_initlock_(void);

AUGSYS_EXTERN int
aug_termlock_(void);
#endif /* AUGSYS_BUILD */

AUGSYS_API void
aug_lock(void);

AUGSYS_API void
aug_unlock(void);

#endif /* AUGSYS_LOCK_H */
