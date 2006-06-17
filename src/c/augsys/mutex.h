/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MUTEX_H
#define AUGSYS_MUTEX_H

#include "augsys/config.h"

typedef struct aug_mutex_* aug_mutex_t;

AUGSYS_API aug_mutex_t
aug_createmutex(void);

AUGSYS_API int
aug_freemutex(aug_mutex_t mutex);

AUGSYS_API int
aug_lockmutex(aug_mutex_t mutex);

AUGSYS_API int
aug_unlockmutex(aug_mutex_t mutex);

#endif /* AUGSYS_MUTEX_H */
