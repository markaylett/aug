/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_LOCK_H
#define AUGCTX_LOCK_H

/**
 * @file augctx/lock.h
 *
 * Global lock functions.
 *
 * Functions in this module may set errno, but never errinfo.
 */

#include "augctx/config.h"

#if defined(AUGCTX_BUILD)
AUG_EXTERNC void
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
