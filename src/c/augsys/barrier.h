/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_BARRIER_H
#define AUGSYS_BARRIER_H

#include "augsys/lock.h"

#if !ENABLE_THREADS
# define AUG_MB()
#else /* ENABLE_THREADS */
# if defined(__GNUC__)
#  if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
#   define AUG_MB() __sync_synchronize()
#  elif defined(__PPC__)
#   define AUG_MB() __asm__ __volatile__("sync":::"memory")
#  elif defined(__i386__) || defined(__i486__) || defined(__i586__) \
    || defined(__i686__) || defined(__x86_64__)
#   define AUG_MB()  __asm__ __volatile__("mfence":::"memory")
#   define AUG_RMB() __asm__ __volatile__("lfence":::"memory")
#   define AUG_WMB() __asm__ __volatile__("sfence":::"memory")
#  else
#   define AUG_MB()                               \
    do {                                          \
        aug_lock();                               \
        aug_unlock();                             \
    } while (0)
#  endif
# elif defined(_MSC_VER)
#  if (_MSC_VER >= 1400)
#   include <intrin.h>
#  else /* _MSC_VER < 1400 */
AUG_EXTERNC void _ReadWriteBarrier(void);
#  endif /* _MSC_VER < 1400 */
#  pragma intrinsic(_ReadWriteBarrier)
# elif defined(__APPLE__) || defined(__MACH__)
#  include <libkern/OSAtomic.h>
#  define AUG_MB() OSMemoryBarrier()
# else
#  define AUG_MB()                               \
    do {                                         \
        aug_lock();                              \
        aug_unlock();                            \
    } while (0)
# endif
#endif/* ENABLE_THREADS */

#if !defined(AUG_RMB)
# define AUG_RMB AUG_MB
#endif /* !AUG_RMB */

#if !defined(AUG_WMB)
# define AUG_WMB AUG_MB
#endif /* !AUG_WMB */

#endif /* AUGSYS_BARRIER_H */
