/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_ERRINFO_H
#define AUGSYS_ERRINFO_H

/**
 * @file augsys/errinfo.h
 *
 * Error handling.
 */

#include "augsys/config.h"
#include "augsys/defs.h"

#include <stdarg.h>

#define AUG_SRCLOCAL 1
#define AUG_SRCPOSIX 2
#define AUG_SRCWIN32 3
#define AUG_SRCDLFCN 4
#define AUG_SRCSSL   5

/**
 * Base value for user-defined exception sources.

 */

#define AUG_SRCUSER  32

/**
 * Common exception code for the #AUG_SRCLOCAL domain.  These codes may also
 * be used as return codes for communicating exception conditions.
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
#define AUG_EINTR    10
#define AUG_EINVAL   11
#define AUG_EIO      12
#define AUG_ELIMIT   13
#define AUG_ENULL    14
#define AUG_EMEMORY  15
#define AUG_EPARSE   16
#define AUG_EPERM    17
#define AUG_ERANGE   18
#define AUG_ESTATE   19
#define AUG_ESUPPORT 20
#define AUG_ETYPE    21
#define AUG_ETIMEOUT 22

struct aug_errinfo {
    char file_[AUG_MAXLINE];
    int line_, src_, num_;
    char desc_[AUG_MAXLINE];
};

/**
 * Due to the boostrapping process implemented by aug_init(), the errinfo
 * facility is not used by the following modules:
 *
 * aug_lock
 * aug_string
 * aug_tls_
 * aug_log
 *
 * All functions in this module set errno, and not errinfo.
 */

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
