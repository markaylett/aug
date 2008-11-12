/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/uio.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsys/posix/uio.c"
#else /* _WIN32 */
# include "augsys/win32/uio.c"
#endif /* _WIN32 */

AUGSYS_API size_t
aug_iovsum(const struct iovec* iov, int size)
{
    int i;
    size_t sum = 0;

    for (i = 0; i < size; ++i)
        sum += iov[i].iov_len;

    return sum;
}
