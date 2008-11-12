/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/lock.h"

#include <limits.h>

#if !defined(_WIN32)
# include "augsys/posix/utility.c"
#else /* _WIN32 */
# include "augsys/win32/utility.c"
#endif /* _WIN32 */

AUGSYS_API void*
aug_memfrob(void* dst, size_t size)
{
    char* ptr = (char*)dst;
    while (size)
        ptr[--size] ^= 42;
    return dst;
}

AUGSYS_API unsigned
aug_nextid(void)
{
    static unsigned id_ = 1;
    unsigned id;

    aug_lock();
    if (id_ == INT_MAX) {
        id_ = 1;
        id = INT_MAX;
    } else
        id = id_++;
    aug_unlock();

    return id;
}
