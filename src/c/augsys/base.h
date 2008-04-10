/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_BASE_H
#define AUGSYS_BASE_H

#include "augsys/config.h"
#include "augsys/types.h"

struct iovec;
struct aug_errinfo;

/**
 * File type structure.
 */

struct aug_fdtype {
    int (*close_)(int);
    ssize_t (*read_)(int, void*, size_t);
    ssize_t (*readv_)(int, const struct iovec*, int);
    ssize_t (*write_)(int, const void*, size_t);
    ssize_t (*writev_)(int, const struct iovec*, int);
    int (*setnonblock_)(int, int);
};

/* The remaining functions will set errinfo on failure. */

/**
 * Get next id.
 *
 * Thread-safe.  Cannot fail.  Will loop when #INT_MAX is reached.
 *
 * @return Next id.
 */

AUGSYS_API int
aug_nextid(void);

/**
 * Associate file descriptor with file type.
 *
 * @param fd File descriptor.
 *
 * @param fdtype File type.
 *
 * @return -1 on failure.
 */

AUGSYS_API int
aug_openfd(int fd, const struct aug_fdtype* fdtype);

/**
 * Associate pair of file descriptors with file type.
 *
 * @param fds File descriptors.
 *
 * @param fdtype File type.
 *
 * @return -1 on failure.
 */

AUGSYS_API int
aug_openfds(int fds[2], const struct aug_fdtype* fdtype);

/**
 * Decrement descriptor's reference count.
 *
 * The descriptor will be closed when the reference count reaches zero.  The
 * close operation is determined by the file type associated with the
 * descriptor.
 *
 * @param fd File descriptor.
 *
 * @return -1 on error.
 */

AUGSYS_API int
aug_releasefd(int fd);

/**
 * Increment descriptor's reference count.
 *
 * @param fd File descriptor.
 *
 * @return -1 on error.
 */

AUGSYS_API int
aug_retainfd(int fd);

/**
 * Associated new type with file descriptor.
 *
 * @return The previous file type.
 */

AUGSYS_API const struct aug_fdtype*
aug_setfdtype(int fd, const struct aug_fdtype* fdtype);

/**
 * Get file type associated with descriptor.
 *
 * @param fd File descriptor.
 *
 * @return File type, or null on error.
 */

AUGSYS_API const struct aug_fdtype*
aug_getfdtype(int fd);

AUGSYS_API struct aug_fdtype*
aug_extfdtype(struct aug_fdtype* derived, const struct aug_fdtype* base);

/**
 * Get posix file type.
 *
 * Cannot fail.
 *
 * @return File type, or null on error.
 */

AUGSYS_API const struct aug_fdtype*
aug_posixfdtype(void);

/**
 * Obtain native file descriptor from posix descriptor.
 */

#if !defined(_WIN32)
# define aug_getosfd(x) (x)
#else /* _WIN32 */
AUGSYS_API intptr_t
aug_getosfd(int fd);
#endif /* _WIN32 */

#endif /* AUGSYS_BASE_H */
