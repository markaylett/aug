/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UTILITY_H
#define AUGSYS_UTILITY_H

#include "augsys/config.h"
#include "augsys/types.h"

/**
 * Scramble or unscramble @a dst buffer.
 *
 * Useful for hiding memory contents, but not true encryption.
 *
 * @param dst The buffer.
 *
 * @param size Size of @a dst buffer.
 *
 * @return @a dst buffer.
 */

AUGSYS_API void*
aug_memfrob(void* dst, size_t size);

/**
 * Get next process-unique id.
 *
 * Thread-safe.  Cannot fail.  Will loop when #INT_MAX is reached.
 *
 * @return Next id.
 */

AUGSYS_API unsigned
aug_nextid(void);

AUGSYS_API long
aug_rand(void);

AUGSYS_API void
aug_srand(unsigned seed);

/**
 * @return Thread identifier, or 0 if the library has been compiled without
 * thread support.
 */

AUGSYS_API unsigned
aug_threadid(void);

#endif /* AUGSYS_UTILITY_H */
