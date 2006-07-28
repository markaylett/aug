/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_BASE_H
#define AUGSYS_BASE_H

#include "augsys/config.h"
#include "augsys/types.h"

struct iovec;
struct aug_errinfo;

struct aug_fddriver {
    int (*close_)(int);
    ssize_t (*read_)(int, void*, size_t);
    ssize_t (*readv_)(int, const struct iovec*, int);
    ssize_t (*write_)(int, const void*, size_t);
    ssize_t (*writev_)(int, const struct iovec*, int);
    int (*setnonblock_)(int, int);
};

/** The aug_init(), aug_term() and aug_atexitinit() functions return errno,
    and not errinfo, on failure.  These functions must be called from the main
    (primary) thread of the process.  They maintain an internal reference
    count that allows them to be called multiple times.  All but the first
    call to aug_init() will simply update the internal reference count.  These
    semantics have been formalised to facilitate initialisation from functions
    such as DllMain. */

AUGSYS_API int
aug_init(struct aug_errinfo* errinfo);

/** For reasons of safety, aug_term() will re-install the default logger.  The
    default logger is garaunteed to be safe even if aug_init() has not been
    called.  Sets errno, and not errinfo. */

AUGSYS_API int
aug_term(void);

AUGSYS_API int
aug_atexitinit(struct aug_errinfo* errinfo);

AUGSYS_API void
aug_exit(int status);

/** The remaining functions will set errinfo on failure. */

AUGSYS_API int
aug_openfd(int fd, const struct aug_fddriver* driver);

AUGSYS_API int
aug_openfds(int fds[2], const struct aug_fddriver* driver);

AUGSYS_API int
aug_releasefd(int fd);

AUGSYS_API int
aug_retainfd(int fd);

AUGSYS_API struct aug_fddriver*
aug_extenddriver(struct aug_fddriver* derived,
                 const struct aug_fddriver* base);

AUGSYS_API int
aug_setfddriver(int fd, const struct aug_fddriver* driver);

AUGSYS_API const struct aug_fddriver*
aug_fddriver(int fd);

AUGSYS_API const struct aug_fddriver*
aug_posixdriver(void);

#endif /* AUGSYS_BASE_H */
