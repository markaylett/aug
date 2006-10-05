/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_LOCK_H
#define AUGSYS_LOCK_H

#include "augsys/config.h"

/**
   All functions in this module set errno, and not errinfo.
*/

#if defined(AUGSYS_BUILD)

# if !defined(AUGSYS_MUTEX_H)
typedef struct aug_mutex_* aug_mutex_t;
# endif /* !AUGSYS_MUTEX_H */

AUGSYS_EXTERN aug_mutex_t
aug_createmutex_(void);

AUGSYS_EXTERN int
aug_freemutex_(aug_mutex_t mutex);

AUGSYS_EXTERN int
aug_lockmutex_(aug_mutex_t mutex);

AUGSYS_EXTERN int
aug_unlockmutex_(aug_mutex_t mutex);

AUGSYS_EXTERN int
aug_initlock_(void);

AUGSYS_EXTERN int
aug_termlock_(void);

#endif /* AUGSYS_BUILD */

/**
   Calls abort() on failure.
*/

AUGSYS_API void
aug_lock(void);

/**
   Calls abort() on failure.
*/

AUGSYS_API void
aug_unlock(void);

#endif /* AUGSYS_LOCK_H */
