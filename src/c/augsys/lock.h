/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#error deprecated
#ifndef AUGSYS_LOCK_H
#define AUGSYS_LOCK_H

/**
 * @file augsys/lock.h
 *
 * Global lock functions.
 *
 * Functions in this module may set errno, but never errinfo.
 */

#include "augsys/config.h"

#if defined(AUGSYS_BUILD)

# if !defined(AUGSYS_MUTEX_H)
typedef struct aug_mutex_* aug_mutex_t;
# endif /* !AUGSYS_MUTEX_H */

AUG_EXTERNC aug_mutex_t
aug_createmutex_(void);

AUG_EXTERNC int
aug_destroymutex_(aug_mutex_t mutex);

AUG_EXTERNC int
aug_lockmutex_(aug_mutex_t mutex);

AUG_EXTERNC int
aug_unlockmutex_(aug_mutex_t mutex);

AUG_EXTERNC int
aug_initlock_(void);

AUG_EXTERNC int
aug_termlock_(void);

#endif /* AUGSYS_BUILD */

/**
 * Obtain global mutex lock.
 *
 * Calls abort() on failure.
 */

AUGSYS_API void
aug_lock(void);

/**
 * Release global mutex lock.
 *
 * Calls abort() on failure.
 */

AUGSYS_API void
aug_unlock(void);

#endif /* AUGSYS_LOCK_H */
