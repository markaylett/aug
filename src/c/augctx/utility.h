/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_UTILITY_H
#define AUGCTX_UTILITY_H

#include "augctx/config.h"

#include <stdarg.h>
#include <stddef.h>

#define aug_die_(file, line, what) \
(fprintf(stderr, "%s:%d: %s\n", file, line, what), fflush(NULL), exit(1))

#define aug_die(what) \
aug_die_(__FILE__, __LINE__, what)

#define aug_check(expr) \
(expr) ? (void)0 : aug_die("check [" #expr "] failed.")

/**
 * Get the default log-level.
 *
 * The log-level specified by the "AUG_LOGLEVEL" environment variable, or
 * #AUG_LOGINFO if not set.
 *
 * @return The log-level.
 */

AUGCTX_API int
aug_loglevel(void);

/**
 * Get timezone offset.
 *
 * Attempt to determine current timezone as seconds west of coordinated
 * universal time.  If a process intends to call chroot(), it should call this
 * function first and cache the result: timezone information may be
 * inaccessible to a jailed process.
 *
 * @param tz Output variable to which timezone will be written.
 *
 * @return The @a tz argument, or null on error.
 */

AUGCTX_API long*
aug_timezone(long* tz);

#endif /* AUGCTX_UTILITY_H */
