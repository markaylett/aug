/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/errinfo.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errno.h"
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

    if (0 > vsnprintf(errinfo->desc_, sizeof(errinfo->desc_), format, args))
        aug_strlcpy(errinfo->desc_, "no message: bad format",
                    sizeof(errinfo->desc_));
}

#if ENABLE_THREADS

# include "augsys/tls_.h"

static int init_ = 0;
static aug_tlskey_t tlskey_;

static struct aug_errinfo*
geterrinfo_(void)
{
    void* errinfo;
    if (-1 == aug_gettlsvalue_(tlskey_, &errinfo))
        errinfo = NULL;

    return errinfo;
}

AUG_EXTERNC struct aug_errinfo*
aug_initerrinfo_(struct aug_errinfo* errinfo)
{
    if (init_) {
        errno = EINVAL;
        return NULL;
    }

    if (-1 == aug_createtlskey_(&tlskey_))
        return NULL;

    if (-1 == aug_settlsvalue_(tlskey_, errinfo)) {
        aug_destroytlskey_(tlskey_);
        return NULL;
    }

    memset(errinfo, 0, sizeof(*errinfo));
    init_ = 1;
    return errinfo;
}

AUG_EXTERNC int
aug_termerrinfo_(void)
{
    if (!init_) {
        errno = EINVAL;
        return -1;
    }

    init_ = 0;
    return aug_destroytlskey_(tlskey_);
}

AUGSYS_API struct aug_errinfo*
aug_initerrinfo(struct aug_errinfo* errinfo)
{
    if (-1 == aug_settlsvalue_(tlskey_, errinfo))
        return NULL;

    memset(errinfo, 0, sizeof(*errinfo));
    return errinfo;
}

#else /* !ENABLE_THREADS */

static struct aug_errinfo* errinfo_ = NULL;

static struct aug_errinfo*
geterrinfo_(void)
{
    return errinfo_ ? errinfo_ : 0;
}

AUG_EXTERNC struct aug_errinfo*
aug_initerrinfo_(struct aug_errinfo* errinfo)
{
    return aug_initerrinfo(errinfo);
}

AUG_EXTERNC int
aug_termerrinfo_(void)
{
    return 0;
}

AUGSYS_API struct aug_errinfo*
aug_initerrinfo(struct aug_errinfo* errinfo)
{
    memset(errinfo, 0, sizeof(*errinfo));
    errinfo_ = errinfo;
    return errinfo;
}

#endif /* !ENABLE_THREADS */

AUGSYS_API int
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                int src, int num, const char* format, va_list args)
{
    if (errinfo || (errinfo = geterrinfo_()))
        vseterrinfo_(errinfo, file, line, src, num, format, args);
    return num;
}

AUGSYS_API int
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               int src, int num, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    aug_vseterrinfo(errinfo, file, line, src, num, format, args);
    va_end(args);
    return num;
}

AUGSYS_API int
aug_setposixerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    int err)
{
    aug_seterrinfo(errinfo, file, line, AUG_SRCPOSIX, err, aug_strerror(err));
    return errno = err;
}

#if defined(_WIN32)
# include "augsys/windows.h"
# include <ctype.h>

AUGSYS_API int
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
    aug_seterrinfo(errinfo, file, line, AUG_SRCWIN32, (int)err,
                   i ? desc : AUG_MSG("no description available"));

    /* Map to errno for completeness. */

    aug_setwin32errno(err);
    return (int)err;
}
#endif /* _WIN32 */

AUGSYS_API int
aug_iserrinfo(const struct aug_errinfo* errinfo, int src, int num)
{
    if (!errinfo)
        errinfo = geterrinfo_();
    return errinfo && errinfo->src_ == src && errinfo->num_ == num;
}

AUGSYS_API const struct aug_errinfo*
aug_geterrinfo(void)
{
    const struct aug_errinfo* errinfo = geterrinfo_();
    return errinfo ? errinfo : &nullinfo_;
}
