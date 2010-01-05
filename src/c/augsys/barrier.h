/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGSYS_BARRIER_H
#define AUGSYS_BARRIER_H

/**
 * @file augsys/barrier.h
 *
 * Portable memory barriers.
 */

#include "augctx/lock.h"

#if defined(_MSC_VER)

# if _MSC_VER >= 1400

#  include <intrin.h>
#  pragma intrinsic(_ReadWriteBarrier)
#  pragma intrinsic(_ReadBarrier)
#  pragma intrinsic(_WriteBarrier)
#  define aug_mb()  _ReadWriteBarrier()
#  define aug_rmb() _ReadBarrier()
#  define aug_wmb() _WriteBarrier()

# else /* _MSC_VER < 1400 */

AUG_EXTERNC void _ReadWriteBarrier(void);
#  pragma intrinsic(_ReadWriteBarrier)
#  define aug_mb()  _ReadWriteBarrier()

# endif /* _MSC_VER < 1400 */

#elif defined(__APPLE__) && defined(__MACH__)

# include <libkern/OSAtomic.h>
# define aug_mb() OSMemoryBarrier()

#elif defined(__GNUC__)

# if !ENABLE_SMP

/**
 * Prevent compiler re-ordering.
 */

#  define aug_mb() __asm__ __volatile__("":::"memory")

# else /* ENABLE_SMP */

#  if defined(__i386__) || defined(__i486__)

#   define aug_mb()  __asm__ __volatile__("lock; addl $0,0(%%esp)":::"memory")
#   define aug_wmb() __asm__ __volatile__("":::"memory")

#  elif defined(__i586__) || defined(__i686__) || defined(__x86_64__)

#   define aug_mb()  __asm__ __volatile__("mfence":::"memory")
#   define aug_rmb() __asm__ __volatile__("lfence":::"memory")
#   define aug_wmb() __asm__ __volatile__("sfence":::"memory")

#  elif defined(__ia64__)

#   define aug_mb()  __asm__ __volatile__("mf":::"memory")

#  elif defined(__alpha__)

#   define aug_mb()  __asm__ __volatile__("mb":::"memory")
#   define aug_wmb() __asm__ __volatile__("wmb":::"memory")

#  elif defined(__PPC__)

#   define aug_mb()  __asm__ __volatile__("sync":::"memory")
#   define aug_wmb() __asm__ __volatile__("eieio":::"memory")

#  elif defined(__sparc__)

#   define aug_mb()  __asm__ __volatile__(
"membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad":::"memory")
#   define aug_rmb() __asm__ __volatile__("membar #LoadLoad":::"memory")
#   define aug_wmb() __asm__ __volatile__("membar #StoreStore":::"memory")

#  elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)

/**
 * Use GCC's intrinsics.
 */

#   define aug_mb()  __sync_synchronize()

#  else

/**
 * Implement in terms of lock.
 */

#   define aug_mb()                               \
    do {                                          \
        aug_lock();                               \
        aug_unlock();                             \
    } while (0)

#  endif

# endif /* ENABLE_SMP */

#else

/**
 * Implement in terms of lock.
 */

# define aug_mb()                               \
    do {                                         \
        aug_lock();                              \
        aug_unlock();                            \
    } while (0)

#endif

/**
 * If not defined, define read barrier in terms of full barrier.
 */

#if !defined(aug_rmb)
# define aug_rmb aug_mb
#endif /* !aug_rmb */

/**
 * If not defined, define write barrier in terms of full barrier.
 */

#if !defined(aug_wmb)
# define aug_wmb aug_mb
#endif /* !aug_wmb */

#endif /* AUGSYS_BARRIER_H */
