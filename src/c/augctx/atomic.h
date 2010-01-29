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
/*3:*/
#line 113 "../../../doc/atomic.w"

#ifndef AUGCTX_ATOMIC_H
#define AUGCTX_ATOMIC_H
/*37:*/
#line 700 "../../../doc/atomic.w"

#define HAVE_AUGCTX_CONFIG_H 1
#if HAVE_AUGCTX_CONFIG_H
# include "augctx/config.h"
#else
/*38:*/
#line 717 "../../../doc/atomic.w"

# if !defined(ENABLE_SMP)
#  define ENABLE_SMP 1
# endif

# if !defined(AUGCTX_SHARED)
#  if defined(DLL_EXPORT) || defined(_WINDLL)
#   define AUGCTX_SHARED
#  endif
# endif

/*:38*/
#line 705 "../../../doc/atomic.w"

/*39:*/
#line 731 "../../../doc/atomic.w"

# if !defined(__cplusplus)
#  define AUG_EXTERNC extern
# else
#  define AUG_EXTERNC extern "C"
# endif

# if defined(__CYGWIN__) || defined(__MINGW32__)
#  define AUG_EXPORT __attribute__ ((dllexport))
#  define AUG_IMPORT __attribute__ ((dllimport))
# elif defined(_MSC_VER)
#  define AUG_EXPORT __declspec(dllexport)
#  define AUG_IMPORT __declspec(dllimport)
# else
#  define AUG_EXPORT
#  define AUG_IMPORT
# endif

# if !defined(AUGCTX_SHARED)
#  define AUGCTX_API AUG_EXTERNC
# else
#  if !defined(AUGCTX_BUILD)
#   define AUGCTX_API AUG_EXTERNC AUG_IMPORT
#  else
#   define AUGCTX_API AUG_EXTERNC AUG_EXPORT
#  endif
# endif

/*:39*/
#line 706 "../../../doc/atomic.w"

#endif
#if defined(_MSC_VER)
/*40:*/
#line 763 "../../../doc/atomic.w"

# if _MSC_VER >= 1400
#  include <intrin.h>
#  pragma intrinsic(_ReadBarrier)
# else
AUG_EXTERNC void _WriteBarrier(void);
AUG_EXTERNC void _ReadWriteBarrier(void);
AUG_EXTERNC long _InterlockedCompareExchange(long volatile*,long,long);
AUG_EXTERNC long _InterlockedExchange(long volatile*,long);
AUG_EXTERNC long _InterlockedExchangeAdd(long volatile*,long);
# endif

# pragma intrinsic(_WriteBarrier)
# pragma intrinsic(_ReadWriteBarrier)
# pragma intrinsic(_InterlockedCompareExchange)
# pragma intrinsic(_InterlockedExchange)
# pragma intrinsic(_InterlockedExchangeAdd)

# define ReadWriteBarrier _ReadWriteBarrier
# define ReadBarrier _ReadBarrier
# define WriteBarrier_ WriteBarrier

# define InterlockedCompareExchange _InterlockedCompareExchange
# define InterlockedExchange _InterlockedExchange
# define InterlockedExchangeAdd _InterlockedExchangeAdd

/*:40*/
#line 709 "../../../doc/atomic.w"

#endif
/*41:*/
#line 792 "../../../doc/atomic.w"

typedef int aug_bool_t;
#if !defined(_MSC_VER)
# include <stdint.h>
#else
typedef __int32 int32_t;
typedef __int64 int64_t;
#endif

/*:41*/
#line 711 "../../../doc/atomic.w"


/*:37*/
#line 116 "../../../doc/atomic.w"

/*28:*/
#line 601 "../../../doc/atomic.w"

#if defined(__APPLE__) && defined(__MACH__)
/*29:*/
#line 645 "../../../doc/atomic.w"

# include <libkern/OSAtomic.h>
# define aug_mb() OSMemoryBarrier()

/*:29*/
#line 603 "../../../doc/atomic.w"

#elif defined(__GNUC__)
# if !ENABLE_SMP

#  define aug_mb() __asm__ __volatile__("":::"memory")
# else
#  if defined(__SSE2__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__)
/*30:*/
#line 652 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("mfence":::"memory")
#   define aug_rmb() __asm__ __volatile__("lfence":::"memory")
#   define aug_wmb() __asm__ __volatile__("sfence":::"memory")

/*:30*/
#line 610 "../../../doc/atomic.w"

#  elif defined(__i386__) || defined(__i486__)
/*31:*/
#line 659 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("lock; addl $0,0(%%esp)":::"memory")
#   define aug_wmb() __asm__ __volatile__("":::"memory")

/*:31*/
#line 612 "../../../doc/atomic.w"

#  elif defined(__ia64__)
/*32:*/
#line 665 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("mf":::"memory")

/*:32*/
#line 614 "../../../doc/atomic.w"

#  elif defined(__alpha__)
/*33:*/
#line 670 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("mb":::"memory")
#   define aug_wmb() __asm__ __volatile__("wmb":::"memory")

/*:33*/
#line 616 "../../../doc/atomic.w"

#  elif defined(__sparc__)
/*34:*/
#line 676 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__(
"membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad":::"memory")
#   define aug_rmb() __asm__ __volatile__("membar #LoadLoad":::"memory")
#   define aug_wmb() __asm__ __volatile__("membar #StoreStore":::"memory")

/*:34*/
#line 618 "../../../doc/atomic.w"

#  elif __GNUC__ >  4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
/*35:*/
#line 684 "../../../doc/atomic.w"

#   define aug_mb()  __sync_synchronize()

/*:35*/
#line 620 "../../../doc/atomic.w"

#  else
#   error No implementation
#  endif
# endif
#elif defined(_MSC_VER)
/*36:*/
#line 689 "../../../doc/atomic.w"

# define aug_mb()  _ReadWriteBarrier()
# if _MSC_VER >= 1400
#  define aug_rmb() _ReadBarrier()
# endif
# define aug_wmb() _WriteBarrier()

/*:36*/
#line 626 "../../../doc/atomic.w"

#else
# error No implementation
#endif



#if !defined(aug_rmb)
# define aug_rmb aug_mb
#endif



#if !defined(aug_wmb)
# define aug_wmb aug_mb
#endif

/*:28*/
#line 117 "../../../doc/atomic.w"

/*5:*/
#line 130 "../../../doc/atomic.w"

AUGCTX_API int32_t
aug_acq32(volatile int32_t*ptr);

AUGCTX_API void*
aug_acqptr(void*volatile*ptr);

/*:5*//*6:*/
#line 141 "../../../doc/atomic.w"

AUGCTX_API void
aug_rel32(volatile int32_t*ptr,int32_t val);

AUGCTX_API void
aug_relptr(void*volatile*ptr,void*val);

/*:6*//*7:*/
#line 154 "../../../doc/atomic.w"

AUGCTX_API aug_bool_t
aug_cas32(volatile int32_t*ptr,int32_t oldval,int32_t newval);

AUGCTX_API aug_bool_t
aug_casptr(void*volatile*ptr,void*oldval,void*newval);

/*:7*//*8:*/
#line 164 "../../../doc/atomic.w"

AUGCTX_API int32_t
aug_tas32(volatile int32_t*ptr,int32_t val);

AUGCTX_API void*
aug_tasptr(void*volatile*ptr,void*val);

/*:8*//*9:*/
#line 174 "../../../doc/atomic.w"

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t delta);

/*:9*/
#line 118 "../../../doc/atomic.w"

#endif

/*:3*/
