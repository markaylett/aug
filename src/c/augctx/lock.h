/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_LOCK_H
#define AUGCTX_LOCK_H

/**
 * @file augctx/lock.h
 *
 * Global lock functions.
 */

#include "augctx/config.h"
#include "augtypes.h"

#if defined(AUGCTX_BUILD)
AUG_EXTERNC aug_result
aug_initlock_(void);
#endif /* AUGCTX_BUILD */

/**
 * Obtain global mutex lock.
 *
 * Calls abort() on failure.
 */

AUGCTX_API void
aug_lock(void);

/**
 * Release global mutex lock.
 *
 * Calls abort() on failure.
 */

AUGCTX_API void
aug_unlock(void);

#endif /* AUGCTX_LOCK_H */
