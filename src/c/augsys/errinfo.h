/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_ERRINFO_H
#define AUGSYS_ERRINFO_H

#include "augsys/config.h"
#include "augsys/defs.h"

#include <stdarg.h>

#define AUG_SRCLOCAL 1
#define AUG_SRCPOSIX 2
#define AUG_SRCWIN32 3
#define AUG_SRCDLFCN 4

/**
   These error constants may also be used as return codes for communicating
   exceptional conditions.
*/

#define AUG_NOERROR  0
#define AUG_ESYSTEM  1

#define AUG_EACCES   2
#define AUG_EAUTH    3
#define AUG_EBOUND   4
#define AUG_EENDOF   5
#define AUG_EEXCEPT  6
#define AUG_EEXIST   7
#define AUG_EFORMAT  8
#define AUG_EINVAL   9
#define AUG_EIO      10
#define AUG_ENULL    11
#define AUG_EPARSE   12
#define AUG_ESUPPORT 13
#define AUG_ETIMEOUT 14

struct aug_errinfo {
    char file_[AUG_MAXLINE];
    int line_, src_, num_;
    char desc_[AUG_MAXLINE];
};

/**
   Due to the boostrapping process implemented by aug_init(), the errinfo
   facility is not used by the following modules:

   aug_lock
   aug_string
   aug_tls_
   aug_log

   All functions in this module set errno, and not errinfo.
*/

#if defined(AUGSYS_BUILD)
AUGSYS_EXTERN int
aug_initerrinfo_(struct aug_errinfo* errinfo);

AUGSYS_EXTERN int
aug_termerrinfo_(void);
#endif /* AUGSYS_BUILD */

AUGSYS_API int
aug_initerrinfo(struct aug_errinfo* errinfo);

AUGSYS_API int
aug_vseterrinfo(const char* file, int line, int src, int num,
                const char* format, va_list args);

AUGSYS_API int
aug_seterrinfo(const char* file, int line, int src, int num,
               const char* format, ...);

AUGSYS_API int
aug_setposixerrinfo(const char* file, int line, int err);

#if defined(_WIN32)
AUGSYS_API int
aug_setwin32errinfo(const char* file, int line, unsigned long err);
#endif /* _WIN32 */

AUGSYS_API const struct aug_errinfo*
aug_geterrinfo(void);

AUGSYS_API int
aug_iserrinfo(int src, int num);

AUGSYS_API int
aug_perrinfo(const char* s);

#define aug_errfile (aug_geterrinfo()->file_)
#define aug_errline (aug_geterrinfo()->line_)
#define aug_errsrc  (aug_geterrinfo()->src_)
#define aug_errnum  (aug_geterrinfo()->num_)
#define aug_errdesc (aug_geterrinfo()->desc_)

#define AUG_PERRINFO(x, s) \
do { \
    if (-1 == x) \
        aug_perrinfo(s); \
} while (0)

#endif /* AUGSYS_ERRINFO_H */
