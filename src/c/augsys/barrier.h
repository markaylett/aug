/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_BARRIER_H
#define AUGSYS_BARRIER_H

#include "augsys/lock.h"

/**
   Portable memory barrier implemented in terms of mutex lock.
*/

#define AUG_MB()                                \
    do {                                        \
        aug_lock();                             \
        aug_unlock();                           \
    } while (0)

#define AUG_RMB AUG_MB
#define AUG_WMB AUG_MB

/**
   Possible specializations for GCC.
*/

#if 0

/* GCC/UP */

#define AUG_MB()                                \
    __asm__ __volatile__("":::"memory")

#define AUG_RMB()                               \
    __asm__ __volatile__("":::"memory")

#define AUG_WMB()                               \
    __asm__ __volatile__("":::"memory")

/* GCC/MP */

#define AUG_MB()                                \
    __asm__ __volatile__("mfence":::"memory")

#define AUG_RMB()                               \
    __asm__ __volatile__("lfence":::"memory")

#define AUG_WMB()                               \
    __asm__ __volatile__("sfence":::"memory")

#endif /* 0 */

#endif /* AUGSYS_BARRIER_H */
