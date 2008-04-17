/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_LOG_H
#define AUGUTIL_LOG_H

/**
 * @file augutil/log.h
 *
 * Common log format.
 */

#include "augutil/config.h"

#include "augctx/log.h"

/**
 * Get textual description of log label.
 *
 * @param level Log level.
 *
 * @return Textual description.
 */

AUGUTIL_API const char*
aug_loglabel(int level);

/**
 * Format log string.
 *
 * @param buf Output buffer.
 *
 * @param n In: size of @a buf.  Out: total number of characters copied
 *
 * @param level Log level.
 *
 * @param format Printf-style format.
 *
 * @param args @a format arguments.
 *
 * @return -1 on error.
 */

AUGUTIL_API int
aug_vformatlog(char* buf, size_t* n, int level, const char* format,
               va_list args);

/**
 * @see aug_vformatlog().
 */

AUGUTIL_API int
aug_formatlog(char* buf, size_t* n, int level, const char* format, ...);

/**
 * The daemon logger.
 *
 * @return An interface to the daemon logger.
 */

AUGUTIL_API aug_log*
aug_getdaemonlog(void);

#endif /* AUGUTIL_LOG_H */
