/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/errinfo.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/string.h"

#include <errno.h>
#include <stdio.h>

#if defined(_WIN32)
# include "augsys/windows.h"
# include <ctype.h>
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

AUGCTX_API void
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                const char* src, int num, const char* format, va_list args)
{
    int ret;

    aug_strlcpy(errinfo->file_, file, sizeof(errinfo->file_));
    errinfo->line_ = line;
    aug_strlcpy(errinfo->src_, src, sizeof(errinfo->src_));
    errinfo->num_ = num;

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = vsnprintf(errinfo->desc_, sizeof(errinfo->desc_), format, args);
    AUG_SNTRUNCF(errinfo->desc_, sizeof(errinfo->desc_), ret);

    if (ret < 0)
        aug_strlcpy(errinfo->desc_, "no message - bad format",
                    sizeof(errinfo->desc_));
}

AUGCTX_API void
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               const char* src, int num, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    aug_vseterrinfo(errinfo, file, line, src, num, format, args);
    va_end(args);
}

AUGCTX_API void
aug_setposixerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    int err)
{
    aug_seterrinfo(errinfo, file, line, "posix", err, strerror(err));
}

#if defined(_WIN32)
AUGCTX_API void
aug_setwin32errinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    unsigned long err)
{
    char desc[AUG_MAXLINE];
    DWORD i = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            desc, sizeof(desc), NULL);

    /* Remove trailing whitespace. */

    while (i && isspace(desc[i - 1]))
        --i;

    /* Remove trailing full-stop. */

    if (i && '.' == desc[i - 1])
        --i;

    desc[i] = '\0';
    aug_seterrinfo(errinfo, file, line, "win32", (int)err,
                   i ? desc : AUG_MSG("no description available"));
}
#endif /* _WIN32 */
