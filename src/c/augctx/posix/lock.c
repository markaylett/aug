/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augctx/utility.h" /* aug_check() */
#include <stdio.h>          /* fprintf() */
#include <stdlib.h>         /* abort() */
#include <pthread.h>

static pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

AUG_EXTERNC aug_result
aug_initlock_(void)
{
    /* Mutex is initialised statically. */

    return AUG_SUCCESS;
}

AUGCTX_API void
aug_lock(void)
{
    aug_check(0 == pthread_mutex_lock(&mutex_));
}

AUGCTX_API void
aug_unlock(void)
{
    aug_check(0 == pthread_mutex_unlock(&mutex_));
}
