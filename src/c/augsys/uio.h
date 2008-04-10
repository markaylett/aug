/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UIO_H
#define AUGSYS_UIO_H

#include "augsys/config.h"
#include "augsys/types.h"

#if !defined(_WIN32)
# include <sys/uio.h>
#else /* _WIN32 */
struct iovec {
    void* iov_base;
    int iov_len;
};
#endif /* _WIN32 */

AUGSYS_API ssize_t
aug_freadv(aug_fd fd, const struct iovec* iov, int size);

AUGSYS_API ssize_t
aug_fwritev(aug_fd fd, const struct iovec* iov, int size);

#endif /* AUGSYS_UIO_H */
