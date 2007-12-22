/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_BARRIER_H
#define AUGSYS_BARRIER_H

/**
 * @file augsys/barrier.h
 *
 * Portable memory barriers.
 */

#include "augsys/lock.h"

#if !ENABLE_SMP
/* Prevent compiler re-ordering. */
# define AUG_MB() __asm__ __volatile__("":::"memory")
#else /* ENABLE_SMP*/
# if defined(__GNUC__)
#  if defined(__i386__) || defined(__i486__)
#   define AUG_MB()  __asm__ __volatile__("lock; addl $0,0(%%esp)":::"memory")
#   define AUG_WMB() __asm__ __volatile__("":::"memory")
#  elif defined(__i586__) || defined(__i686__) || defined(__x86_64__)
#   define AUG_MB()  __asm__ __volatile__("mfence":::"memory")
#   define AUG_RMB() __asm__ __volatile__("lfence":::"memory")
#   define AUG_WMB() __asm__ __volatile__("sfence":::"memory")
#  elif defined(__ia64__)
#   define AUG_MB()  __asm__ __volatile__("mf":::"memory")
#  elif defined(__alpha__)
#   define AUG_MB()  __asm__ __volatile__("mb":::"memory")
#   define AUG_WMB() __asm__ __volatile__("wmb":::"memory")
#  elif defined(__PPC__)
#   define AUG_MB()  __asm__ __volatile__("sync":::"memory")
#   define AUG_WMB() __asm__ __volatile__("eieio":::"memory")
#  elif defined(__sparc__)
#   define AUG_MB()  __asm__ __volatile__(
"membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad":::"memory")
#   define AUG_RMB() __asm__ __volatile__("membar #LoadLoad":::"memory")
#   define AUG_WMB() __asm__ __volatile__("membar #StoreStore":::"memory")
#  elif (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
/* Use GCC's intrinsics. */
#   define AUG_MB()  __sync_synchronize()
#  else
/* Implement in terms of lock. */
#   define AUG_MB()                               \
    do {                                          \
        aug_lock();                               \
        aug_unlock();                             \
    } while (0)
#  endif
# elif defined(_MSC_VER)
#  if (_MSC_VER >= 1400)
#   include <intrin.h>
#   pragma intrinsic(_ReadWriteBarrier)
#   pragma intrinsic(_ReadBarrier)
#   pragma intrinsic(_WriteBarrier)
#   define AUG_MB()  _ReadWriteBarrier()
#   define AUG_RMB() _ReadBarrier()
#   define AUG_WMB() _WriteBarrier()
#  else /* _MSC_VER < 1400 */
AUG_EXTERNC void _ReadWriteBarrier(void);
#   pragma intrinsic(_ReadWriteBarrier)
#   define AUG_MB()  _ReadWriteBarrier()
#  endif /* _MSC_VER < 1400 */
# elif defined(__APPLE__) || defined(__MACH__)
#  include <libkern/OSAtomic.h>
#  define AUG_MB() OSMemoryBarrier()
# else
/* Implement in terms of posix lock. */
#  define AUG_MB()                               \
    do {                                         \
        aug_lock();                              \
        aug_unlock();                            \
    } while (0)
# endif
#endif /* ENABLE_SMP */

/**
 * If not defined, define read barrier in terms of full barrier.
 */

#if !defined(AUG_RMB)
# define AUG_RMB AUG_MB
#endif /* !AUG_RMB */

/**
 * If not defined, define write barrier in terms of full barrier.
 */

#if !defined(AUG_WMB)
# define AUG_WMB AUG_MB
#endif /* !AUG_WMB */

#endif /* AUGSYS_BARRIER_H */
