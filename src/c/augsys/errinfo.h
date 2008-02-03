/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_ERRINFO_H
#define AUGSYS_ERRINFO_H

/**
 * @file augsys/errinfo.h
 *
 * Error handling.
 *
 * Due to the boostrapping process implemented by aug_init(), the errinfo
 * facility is not used in the following modules:
 *
 * @li augsys/lock.h
 * @li augsys/string.h
 * @li augsys/tls_.h
 * @li augsys/log.h
 *
 * Functions in this module may set errno, but never errinfo.
 */

/**
 * @defgroup ErrInfo Error Info
 */

#include "augsys/config.h"
#include "augctx/defs.h"

#include <stdarg.h>

/**
 * @defgroup ErrInfoSource Error Source
 *
 * @ingroup ErrInfo
 *
 * @{
 */

#define AUG_SRCLOCAL 1
#define AUG_SRCPOSIX 2
#define AUG_SRCWIN32 3
#define AUG_SRCDLFCN 4
#define AUG_SRCSSL   5

/**
 * Base value for user-defined exception sources.
 */

#define AUG_SRCUSER  32

/** @} */

/**
 * @defgroup ErrInfoLocal Local Error
 *
 * @ingroup ErrInfo
 *
 * Common exception code for the #AUG_SRCLOCAL domain.  These codes may also
 * be used as return codes for communicating exception conditions.
 *
 * @{
 */

#define AUG_NOERROR  0
#define AUG_ESYSTEM  1

#define AUG_EASSERT  2
#define AUG_EAUTH    3
#define AUG_ECONFIG  4
#define AUG_EDOMAIN  5
#define AUG_EENDOF   6
#define AUG_EEXCEPT  7
#define AUG_EEXIST   8
#define AUG_EFORMAT  9
#define AUG_EINVAL   10
#define AUG_EIO      11
#define AUG_ELIMIT   12
#define AUG_ENULL    13
#define AUG_EMEMORY  14
#define AUG_EPARSE   15
#define AUG_EPERM    16
#define AUG_ERANGE   17
#define AUG_ESTATE   18
#define AUG_ESUPPORT 19
#define AUG_ETYPE    20
#define AUG_ETIMEOUT 21

/** @} */

struct aug_errinfo {
    char file_[AUG_MAXLINE];
    int line_, src_, num_;
    char desc_[AUG_MAXLINE];
};

#if defined(AUGSYS_BUILD)
AUG_EXTERNC struct aug_errinfo*
aug_initerrinfo_(struct aug_errinfo* errinfo);

AUG_EXTERNC int
aug_termerrinfo_(void);
#endif /* AUGSYS_BUILD */

/**
 * Initialise errinfo for secondary thread.
 */

AUGSYS_API struct aug_errinfo*
aug_initerrinfo(struct aug_errinfo* errinfo);

AUGSYS_API int
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                int src, int num, const char* format, va_list args);

AUGSYS_API int
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               int src, int num, const char* format, ...);

AUGSYS_API int
aug_setposixerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    int err);

#if defined(_WIN32)
AUGSYS_API int
aug_setwin32errinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    unsigned long err);
#endif /* _WIN32 */

AUGSYS_API int
aug_iserrinfo(const struct aug_errinfo* errinfo, int src, int num);

AUGSYS_API const struct aug_errinfo*
aug_geterrinfo(void);

#define aug_errfile (aug_geterrinfo()->file_)
#define aug_errline (aug_geterrinfo()->line_)
#define aug_errsrc  (aug_geterrinfo()->src_)
#define aug_errnum  (aug_geterrinfo()->num_)
#define aug_errdesc (aug_geterrinfo()->desc_)

#define AUG_PERRINFO(x, e, s)                    \
    do {                                         \
        if (-1 == x)                             \
            aug_perrinfo(e, s);                  \
    } while (0)

#endif /* AUGSYS_ERRINFO_H */
