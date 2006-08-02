/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#undef __STRICT_ANSI__ /* vsnprintf() */
#define AUGSYS_BUILD
#include "augsys/errinfo.h"

static const char rcsid[] = "$Id:$";

#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/string.h"

#include <stdio.h>

#if defined(_WIN32)
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

static const struct aug_errinfo nullinfo_ = { "", 0, 0, 0, "" };

static void
vseterrinfo_(struct aug_errinfo* errinfo, const char* file, int line, int src,
             int num, const char* format, va_list args)
{
    aug_strlcpy(errinfo->file_, file, sizeof(errinfo->file_));

    errinfo->line_ = line;
    errinfo->src_ = src;
    errinfo->num_ = num;

    if (0 > vsnprintf(errinfo->desc_, sizeof(errinfo->desc_),
                      format, args))
        aug_strlcpy(errinfo->desc_, "no message: bad format",
                    sizeof(errinfo->desc_));
}

static void
vwritelog_(const char* file, int line, int src, int num, const char* format,
           va_list args)
{
    struct aug_errinfo errinfo;

    aug_strlcpy(errinfo.file_, file, sizeof(errinfo.file_));
    errinfo.line_ = line;
    errinfo.src_ = src;
    errinfo.num_ = num;

    if (0 > vsnprintf(errinfo.desc_, sizeof(errinfo.desc_), format, args))
        aug_strlcpy(errinfo.desc_, "no message: bad format",
                    sizeof(errinfo.desc_));

    aug_error("%s: %d: %s", errinfo.file_, (int)errinfo.line_, errinfo.desc_);
}

#if !defined(_MT)

static struct aug_errinfo* errinfo_ = NULL;

static const struct aug_errinfo*
geterrinfo_(void)
{
    return errinfo_ ? errinfo_ : 0;
}

AUGSYS_EXTERN int
aug_initerrinfo_(struct aug_errinfo* errinfo)
{
    return aug_initerrinfo(errinfo);
}

AUGSYS_EXTERN int
aug_termerrinfo_(void)
{
    return 0;
}

AUGSYS_API int
aug_initerrinfo(struct aug_errinfo* errinfo)
{
    memset(errinfo, 0, sizeof(*errinfo));
    errinfo_ = errinfo;
    return 0;
}

AUGSYS_API int
aug_vseterrinfo(const char* file, int line, int src, int num,
                const char* format, va_list args)
{
    if (errinfo_)
        vseterrinfo_(errinfo_, file, line, src, num, format, args);
    else
        vwritelog_(file, line, src, num, format, args);
    return num;
}

AUGSYS_API int
aug_seterrinfo(const char* file, int line, int src, int num,
               const char* format, ...)
{
    va_list args;
    va_start(args, format);
    aug_vseterrinfo(file, line, src, num, format, args);
    va_end(args);
    return num;
}

#else /* _MT */

# include "augsys/tls_.h"

static int init_ = 0;
static aug_tlskey_t tlskey_;

static const struct aug_errinfo*
geterrinfo_(void)
{
    void* errinfo;
    if (!init_ || -1 == aug_gettlsvalue_(tlskey_, &errinfo))
        return NULL;

    return errinfo;
}

AUGSYS_EXTERN int
aug_initerrinfo_(struct aug_errinfo* errinfo)
{
    if (init_) {
        errno = EINVAL;
        return -1;
    }

    if (-1 == aug_createtlskey_(&tlskey_))
        return -1;

    if (-1 == aug_settlsvalue_(tlskey_, errinfo)) {
        aug_freetlskey_(tlskey_);
        return -1;
    }

    memset(errinfo, 0, sizeof(*errinfo));
    init_ = 1;
    return 0;
}

AUGSYS_EXTERN int
aug_termerrinfo_(void)
{
    if (!init_) {
        errno = EINVAL;
        return -1;
    }

    init_ = 0;
    return aug_freetlskey_(tlskey_);
}

AUGSYS_API int
aug_initerrinfo(struct aug_errinfo* errinfo)
{
    if (!init_) {
        errno = EINVAL;
        return -1;
    }

    if (-1 == aug_settlsvalue_(tlskey_, errinfo))
        return -1;

    memset(errinfo, 0, sizeof(*errinfo));
    return 0;
}

AUGSYS_API int
aug_vseterrinfo(const char* file, int line, int src, int num,
                const char* format, va_list args)
{
    void* value;
    if (init_ && -1 != aug_gettlsvalue_(tlskey_, &value) && value)
        vseterrinfo_((struct aug_errinfo*)value, file, line, src, num,
                     format, args);
    else
        vwritelog_(file, line, src, num, format, args);
    return num;
}

AUGSYS_API int
aug_seterrinfo(const char* file, int line, int src, int num,
               const char* format, ...)
{
    va_list args;
    va_start(args, format);
    aug_vseterrinfo(file, line, src, num, format, args);
    va_end(args);
    return num;
}

#endif /* _MT */

AUGSYS_API int
aug_setposixerrinfo(const char* file, int line, int err)
{
    aug_seterrinfo(file, line, AUG_SRCPOSIX, err, aug_strerror(err));
    return errno = err;
}

#if defined(_WIN32)

#include "augsys/windows.h"

#include <ctype.h>

AUGSYS_API int
aug_setwin32errinfo(const char* file, int line, unsigned long err)
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
    aug_seterrinfo(file, line, AUG_SRCWIN32, (int)err,
                   i ? desc : AUG_MSG("no description available"));

    /* Map to errno for completeness. */

    aug_setwin32errno(err);
    return (int)err;
}

#endif /* _WIN32 */

AUGSYS_API const struct aug_errinfo*
aug_geterrinfo(void)
{
    const struct aug_errinfo* errinfo = geterrinfo_();
    return errinfo ? errinfo : &nullinfo_;
}

AUGSYS_API int
aug_iserrinfo(int src, int num)
{
    const struct aug_errinfo* errinfo = geterrinfo_();
    return errinfo && errinfo->src_ == src && errinfo->num_ == num;
}

AUGSYS_API int
aug_perrinfo(const char* s)
{
    const struct aug_errinfo* errinfo = geterrinfo_();
    const char* file;
    if (!errinfo || 0 == errinfo->num_) {
        aug_error("%s: no description available", s);
        return 0;
    }

    for (file = errinfo->file_;; ++file)
        switch (*file) {
        case '.':
        case '/':
        case '\\':
            break;
        default:
            goto done;
        }
 done:
    return aug_error("%s: [src=%d, num=0x%.8x (%d)] %s at %s line %d.", s,
                     errinfo->src_, (int)errinfo->num_, (int)errinfo->num_,
                     errinfo->desc_, file, (int)errinfo->line_);
}
