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

#endif /* AUGSYS_BASE_H */
