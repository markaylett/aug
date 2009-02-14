/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGSYS_UIO_H
#define AUGSYS_UIO_H

#include "augsys/config.h"
#include "augsys/types.h"

#include "augtypes.h"

#if !defined(_WIN32)
# include <sys/uio.h>
#else /* _WIN32 */
struct iovec {
    void* iov_base;
    int iov_len;
};
#endif /* _WIN32 */

AUGSYS_API aug_rsize
aug_freadv(aug_fd fd, const struct iovec* iov, int size);

AUGSYS_API aug_rsize
aug_fwritev(aug_fd fd, const struct iovec* iov, int size);

/**
 * Sum of all iov_len values.
 *
 * @param iov Array.
 * @param size Array elements.
 *
 * @return Sum.
 */

AUGSYS_API size_t
aug_iovsum(const struct iovec* iov, int size);

#endif /* AUGSYS_UIO_H */
