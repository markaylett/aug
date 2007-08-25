/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_BASE_H
#define AUGSYS_BASE_H

#include "augsys/config.h"
#include "augsys/types.h"

struct iovec;
struct aug_errinfo;

struct aug_fdtype {
    int (*close_)(int);
    ssize_t (*read_)(int, void*, size_t);
    ssize_t (*readv_)(int, const struct iovec*, int);
    ssize_t (*write_)(int, const void*, size_t);
    ssize_t (*writev_)(int, const struct iovec*, int);
    int (*setnonblock_)(int, int);
};

#if defined(AUGSYS_BUILD)

/**
   Set log-level.

   \return previous log-level.
 */

AUG_EXTERNC int
aug_setloglevel_(int loglevel);

/**
   Set logger.

   \return previous logger.
 */

AUG_EXTERNC aug_logger_t
aug_setlogger_(aug_logger_t logger);

/**
   Obtain logger information.
*/

AUG_EXTERNC int
aug_loglevel_(aug_logger_t* logger);
#endif /* AUGSYS_BUILD */

/**
   Initialise use of aug libraries.

   On failure, aug_init(), aug_term() and aug_atexitinit() functions set
   errno, and not #aug_errinfo.  These functions must be called from the main
   (primary) thread of the process.  They maintain an internal reference count
   that allows them to be called multiple times.  All but the first call to
   aug_init() simply updates the reference count.  These semantics have been
   formalised to facilitate initialisation from functions such as DllMain.
*/

AUGSYS_API int
aug_init(struct aug_errinfo* errinfo);

/**
   Terminate use of aug libraries.

   For safety reasons, aug_term() will re-install the default logger.  The
   default logger is garaunteed to be safe even if aug_init() has not been
   called.  This function sets errno, and not errinfo, on failure.
*/

AUGSYS_API int
aug_term(void);

AUGSYS_API int
aug_atexitinit(struct aug_errinfo* errinfo);

/**
   Exit process.

   Force termination of aug libraries, regardless of reference count, before
   calling exit().

   \param status The exit status.
*/

AUGSYS_API void
aug_exit(int status);

/* The remaining functions will set errinfo on failure. */

/**
   Timezone offset.

   Seconds west of coordinated universal time.
*/

AUGSYS_API long
aug_timezone(void);

AUGSYS_API int
aug_nextid(void);

AUGSYS_API int
aug_openfd(int fd, const struct aug_fdtype* fdtype);

AUGSYS_API int
aug_openfds(int fds[2], const struct aug_fdtype* fdtype);

AUGSYS_API int
aug_releasefd(int fd);

AUGSYS_API int
aug_retainfd(int fd);

/**
   Associated new type with file descriptor.

   \return The previous file type.
*/

AUGSYS_API const struct aug_fdtype*
aug_setfdtype(int fd, const struct aug_fdtype* fdtype);

AUGSYS_API const struct aug_fdtype*
aug_getfdtype(int fd);

AUGSYS_API struct aug_fdtype*
aug_extfdtype(struct aug_fdtype* derived, const struct aug_fdtype* base);

AUGSYS_API const struct aug_fdtype*
aug_posixfdtype(void);

/**
   Obtain native file descriptor from posix descriptor.
*/

#if !defined(_WIN32)
# define aug_getosfd(x) (x)
#else /* _WIN32 */
AUGSYS_API intptr_t
aug_getosfd(int fd);
#endif /* _WIN32 */

#endif /* AUGSYS_BASE_H */
