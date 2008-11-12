/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsys/windows.h"

#include <stdlib.h>

AUGSYS_API long
aug_rand(void)
{
    return (long)rand();
}

AUGSYS_API void
aug_srand(unsigned seed)
{
    srand(seed);
}

AUGSYS_API unsigned
aug_threadid(void)
{
#if ENABLE_THREADS
    return (unsigned)GetCurrentThreadId();
#else /* !ENABLE_THREADS */
    return 0;
#endif /* !ENABLE_THREADS */
}
