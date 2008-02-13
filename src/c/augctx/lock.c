/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/lock.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_THREADS
# if !defined(_WIN32)
#  include "augctx/posix/lock.c"
# else /* _WIN32 */
#  include "augctx/win32/lock.c"
# endif /* _WIN32 */
#else /* !ENABLE_THREADS */

/* Single-threaded stubs. */

AUG_EXTERNC aug_result
aug_initlock_(void)
{
    return AUG_TRUE;
}

AUGCTX_API void
aug_lock(void)
{
}

AUGCTX_API void
aug_unlock(void)
{
}

#endif /* !ENABLE_THREADS */
