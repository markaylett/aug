/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

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
