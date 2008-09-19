/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_ERRINFO_H
#define AUGCTX_ERRINFO_H

#include "augctx/config.h"

#include "augtypes.h" /* aug_bool */

#include <stdarg.h>

struct aug_errinfo {
    char file_[512];
    int line_;
    char src_[32];
    int num_;
    char desc_[512];
};

AUGCTX_API void
aug_clearerrinfo(struct aug_errinfo* errinfo);

AUGCTX_API void
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                const char* src, int num, const char* format, va_list args);

AUGCTX_API void
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               const char* src, int num, const char* format, ...);

/**
 * @return Exception code or zero.
 */

AUGCTX_API aug_result
aug_setposixerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    int num);

#if defined(_WIN32)

/**
 * @return Exception code or zero.
 */

AUGCTX_API aug_result
aug_setwin32errinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    unsigned long num);
#endif /* _WIN32 */

/**
 * Get equivalent errno value.
 *
 * @param errinfo Error info.
 *
 * @return Errno value or zero.
 */

AUGCTX_API int
aug_errno(const struct aug_errinfo* errinfo);

#endif /* AUGCTX_ERRINFO_H */
