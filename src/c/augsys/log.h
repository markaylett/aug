/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_LOG_H
#define AUGSYS_LOG_H

#include "augsys/config.h"
#include "augsys/types.h"

#include <stdarg.h>

/**
   All functions in this module set errno, and not errinfo.
*/

typedef int (*aug_logger_t)(int, const char*, va_list);

AUGSYS_API int
aug_stdiologger(int loglevel, const char* format, va_list args);

AUGSYS_API void
aug_setloglevel(int loglevel);

/**
   If the logger argument is NULL, the default logger is re-installed.
*/

AUGSYS_API aug_logger_t
aug_setlogger(aug_logger_t logger);

AUGSYS_API int
aug_loglevel(void);

AUGSYS_API int
aug_vwritelog(int loglevel, const char* format, va_list args);

AUGSYS_API int
aug_writelog(int loglevel, const char* format, ...);

/**
   The following functions are essentially convenience wrappers around
   aug_vwritelog().
*/

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

/**
   Guidelines for debug-level use:

   aug_debug0() - user applications;
   aug_debug1() - user applications;
   aug_debug2() - aug applications (such as daug and mar);
   aug_debug3() - aug libraries.

   Note: further levels can be used by calling aug_writelog() directly.
*/

AUGSYS_API int
aug_debug0(const char* format, ...);

AUGSYS_API int
aug_debug1(const char* format, ...);

AUGSYS_API int
aug_debug2(const char* format, ...);

AUGSYS_API int
aug_debug3(const char* format, ...);

#if !defined(NDEBUG)
# define AUG_DEBUG0 aug_debug0
# define AUG_DEBUG1 aug_debug1
# define AUG_DEBUG2 aug_debug2
# define AUG_DEBUG3 aug_debug3
#else /* NDEBUG */
# define AUG_DEBUG0 aug_debug0
# define AUG_DEBUG1 1 ? (void)0 : (void)aug_debug1
# define AUG_DEBUG2 1 ? (void)0 : (void)aug_debug2
# define AUG_DEBUG3 1 ? (void)0 : (void)aug_debug3
#endif /* NDEBUG */

#endif /* AUGSYS_LOG_H */
