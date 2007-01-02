/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_LOG_H
#define AUGUTIL_LOG_H

#include "augutil/config.h"

#include "augsys/types.h"

#include <stdarg.h>

AUGUTIL_API const char*
aug_loglabel(int loglevel);

AUGUTIL_API int
aug_vformatlog(char* buf, size_t* n, int loglevel, const char* format,
               va_list args);

AUGUTIL_API int
aug_formatlog(char* buf, size_t* n, int loglevel, const char* format, ...);

AUGUTIL_API int
aug_daemonlogger(int loglevel, const char* format, va_list args);

#endif /* AUGUTIL_LOG_H */
