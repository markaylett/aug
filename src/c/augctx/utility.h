/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_UTILITY_H
#define AUGCTX_UTILITY_H

#include "augctx/config.h"

#include <stdarg.h>
#include <stddef.h>

struct aug_errinfo;

AUGCTX_API size_t
aug_strlcpy(char* dst, const char* src, size_t size);

AUGCTX_API void
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                const char* src, int num, const char* format, va_list args);

AUGCTX_API void
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               const char* src, int num, const char* format, ...);

AUGCTX_API int
aug_loglevel(void);

AUGCTX_API long*
aug_timezone(long* tz);

#endif /* AUGCTX_UTILITY_H */
