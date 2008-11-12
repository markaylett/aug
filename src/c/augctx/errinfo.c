/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/errinfo.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/errno.h"
#include "augctx/string.h"

#include <errno.h>
#include <stdio.h>

#if defined(_WIN32)
# if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif /* !WIN32_LEAN_AND_MEAN */
# include <windows.h>
# include <ctype.h>
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

static void
seterrinfo_(struct aug_errinfo* errinfo, const char* file, int line,
            const char* src, int num, const char* desc)
{
    aug_strlcpy(errinfo->file_, file, sizeof(errinfo->file_));
    errinfo->line_ = line;
    aug_strlcpy(errinfo->src_, src, sizeof(errinfo->src_));
    errinfo->num_ = num;
    aug_strlcpy(errinfo->desc_, desc, sizeof(errinfo->desc_));
}

static void
vseterrinfo_(struct aug_errinfo* errinfo, const char* file, int line,
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
aug_clearerrinfo(struct aug_errinfo* errinfo)
{
    errinfo->file_[0] = '\0';
    errinfo->line_ = 0;
    errinfo->src_[0] = '\0';
    errinfo->num_ = 0;
    errinfo->desc_[0] = '\0';
}

AUGCTX_API void
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                const char* src, int num, const char* format, va_list args)
{
    if (!num) {
        aug_clearerrinfo(errinfo);
        return;
    }

    vseterrinfo_(errinfo, file, line, src, num, format, args);
}

AUGCTX_API void
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               const char* src, int num, const char* format, ...)
{
    va_list args;

    if (!num) {
        aug_clearerrinfo(errinfo);
        return;
    }

    va_start(args, format);
    vseterrinfo_(errinfo, file, line, src, num, format, args);
    va_end(args);
}

AUGCTX_API aug_result
aug_setposixerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    int num)
{
    if (!num) {
        aug_clearerrinfo(errinfo);
        return AUG_SUCCESS;
    }

    seterrinfo_(errinfo, file, line, "posix", num, strerror(num));

    /* Map to exception code. */

    switch (num) {
    case EINTR:
        return AUG_FAILINTR;
    case EWOULDBLOCK:
        return AUG_FAILBLOCK;
    }
    return AUG_FAILERROR;
}

#if defined(_WIN32)
AUGCTX_API aug_result
aug_setwin32errinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    unsigned long num)
{
    char desc[AUG_MAXLINE];
    DWORD i;

    if (!num) {
        aug_clearerrinfo(errinfo);
        return AUG_SUCCESS;
    }

    i = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, num,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      desc, sizeof(desc), NULL);

    /* Remove trailing whitespace. */

    while (i && isspace(desc[i - 1]))
        --i;

    /* Remove trailing full-stop. */

    if (i && '.' == desc[i - 1])
        --i;

    desc[i] = '\0';

    seterrinfo_(errinfo, file, line, "win32", (int)num,
                i ? desc : AUG_MSG("no description available"));

    /* Map to exception code. */

    switch (num) {
    case WSAEINTR:
        return AUG_FAILINTR;
    case WSAEWOULDBLOCK:
        return AUG_FAILBLOCK;
    }
    return AUG_FAILERROR;
}
#endif /* _WIN32 */

AUGCTX_API int
aug_errno(const struct aug_errinfo* errinfo)
{
    if (0 == aug_strncasecmp(errinfo->src_, "posix", sizeof(errinfo->src_)))
        return errinfo->num_;

#if defined(_WIN32)
    if (0 == aug_strncasecmp(errinfo->src_, "win32", sizeof(errinfo->src_)))
        return aug_win32posix(errinfo->num_);
#endif /* _WIN32 */

    return 0;
}
