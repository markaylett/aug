/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_LOG_H
#define AUGSYS_LOG_H

#include "augsys/config.h"
#include "augsys/types.h"

#include <stdarg.h>

/** All functions in this module set errno, and not errinfo. */

typedef int (*aug_logger_t)(int, const char*, va_list);

AUGSYS_API int
aug_stdiologger(int loglevel, const char* format, va_list args);

AUGSYS_API void
aug_setloglevel(int loglevel);

/** If the logger argument is NULL, the default logger is re-installed. */

AUGSYS_API aug_logger_t
aug_setlogger(aug_logger_t logger);

AUGSYS_API int
aug_loglevel(void);

AUGSYS_API int
aug_vwritelog(int loglevel, const char* format, va_list args);

AUGSYS_API int
aug_writelog(int loglevel, const char* format, ...);

/** The following functions are essentially convenience wrappers around
    aug_vwritelog(). */

AUGSYS_API int
aug_crit(const char* format, ...);

AUGSYS_API int
aug_error(const char* format, ...);

AUGSYS_API int
aug_warn(const char* format, ...);

AUGSYS_API int
aug_notice(const char* format, ...);

AUGSYS_API int
aug_info(const char* format, ...);

AUGSYS_API int
aug_debug(const char* format, ...);

#if !defined(NDEBUG)
# define AUG_DEBUG aug_debug
#else /* NDEBUG */
# define AUG_DEBUG 1 ? ((void)0) : aug_debug
#endif /* NDEBUG */

#endif /* AUGSYS_LOG_H */
