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

#include "augsys/types.h" /* size_t */

/**
 * Get textual description of log label.
 *
 * @param loglevel Log level.
 *
 * @return Textual description.
 */

AUGUTIL_API const char*
aug_loglabel(int loglevel);

/**
 * Format log string.
 *
 * @param buf Output buffer.
 *
 * @param n In: size of @a buf.  Out: total number of characters copied
 *
 * @param loglevel Log level.
 *
 * @param format Printf-style format.
 *
 * @param args @a format arguments.
 *
 * @return -1 on error.
 */

AUGUTIL_API int
aug_vformatlog(char* buf, size_t* n, int loglevel, const char* format,
               va_list args);

/**
 * @see aug_vformatlog().
 */

AUGUTIL_API int
aug_formatlog(char* buf, size_t* n, int loglevel, const char* format, ...);

/**
 * Logger function intended for use with aug_setlogger().
 *
 * Each log line consists of time, label, thread-id and message.
 *
 * @param loglevel Log level.
 *
 * @param format Printf-style format.
 *
 * @param args @a format arguments.
 *
 * @return -1 on error.
 */

AUGUTIL_API int
aug_daemonlogger(int loglevel, const char* format, va_list args);

#endif /* AUGUTIL_LOG_H */
