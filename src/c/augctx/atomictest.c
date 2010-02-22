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
/*2:*/
#line 106 "../../../doc/atomic.w"

/*3:*/
#line 114 "../../../doc/atomic.w"

#ifndef AUGCTX_ATOMIC_H
#define AUGCTX_ATOMIC_H
/*37:*/
#line 701 "../../../doc/atomic.w"

#define HAVE_AUGCTX_CONFIG_H 0
#if HAVE_AUGCTX_CONFIG_H
# include "augctx/config.h"
#else
/*38:*/
#line 718 "../../../doc/atomic.w"

# if !defined(ENABLE_SMP)
#  define ENABLE_SMP 1
# endif

# if !defined(AUGCTX_SHARED)
#  if defined(DLL_EXPORT) || defined(_WINDLL)
#   define AUGCTX_SHARED
#  endif
# endif

/*:38*/
#line 706 "../../../doc/atomic.w"

/*39:*/
#line 732 "../../../doc/atomic.w"

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
#line 707 "../../../doc/atomic.w"

#endif
#if defined(_MSC_VER)
/*40:*/
#line 764 "../../../doc/atomic.w"

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
#line 710 "../../../doc/atomic.w"

#endif
/*41:*/
#line 793 "../../../doc/atomic.w"

typedef int aug_bool_t;
#if !defined(_MSC_VER)
# include <stdint.h>
#else
typedef __int32 int32_t;
typedef __int64 int64_t;
#endif

/*:41*/
#line 712 "../../../doc/atomic.w"


/*:37*/
#line 117 "../../../doc/atomic.w"

/*28:*/
#line 602 "../../../doc/atomic.w"

#if defined(__APPLE__) && defined(__MACH__)
/*29:*/
#line 646 "../../../doc/atomic.w"

# include <libkern/OSAtomic.h>
# define aug_mb() OSMemoryBarrier()

/*:29*/
#line 604 "../../../doc/atomic.w"

#elif defined(__GNUC__)
# if !ENABLE_SMP

#  define aug_mb() __asm__ __volatile__("":::"memory")
# else
#  if defined(__SSE2__) || defined(__x86_64__)
/*30:*/
#line 653 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("mfence":::"memory")
#   define aug_rmb() __asm__ __volatile__("lfence":::"memory")
#   define aug_wmb() __asm__ __volatile__("sfence":::"memory")

/*:30*/
#line 611 "../../../doc/atomic.w"

#  elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
/*31:*/
#line 660 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("lock; addl $0,0(%%esp)":::"memory")
#   define aug_wmb() __asm__ __volatile__("":::"memory")

/*:31*/
#line 613 "../../../doc/atomic.w"

#  elif defined(__ia64__)
/*32:*/
#line 666 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("mf":::"memory")

/*:32*/
#line 615 "../../../doc/atomic.w"

#  elif defined(__alpha__)
/*33:*/
#line 671 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__("mb":::"memory")
#   define aug_wmb() __asm__ __volatile__("wmb":::"memory")

/*:33*/
#line 617 "../../../doc/atomic.w"

#  elif defined(__sparc__)
/*34:*/
#line 677 "../../../doc/atomic.w"

#   define aug_mb()  __asm__ __volatile__(
"membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad":::"memory")
#   define aug_rmb() __asm__ __volatile__("membar #LoadLoad":::"memory")
#   define aug_wmb() __asm__ __volatile__("membar #StoreStore":::"memory")

/*:34*/
#line 619 "../../../doc/atomic.w"

#  elif __GNUC__ >  4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
/*35:*/
#line 685 "../../../doc/atomic.w"

#   define aug_mb()  __sync_synchronize()

/*:35*/
#line 621 "../../../doc/atomic.w"

#  else
#   error No implementation
#  endif
# endif
#elif defined(_MSC_VER)
/*36:*/
#line 690 "../../../doc/atomic.w"

# define aug_mb()  _ReadWriteBarrier()
# if _MSC_VER >= 1400
#  define aug_rmb() _ReadBarrier()
# endif
# define aug_wmb() _WriteBarrier()

/*:36*/
#line 627 "../../../doc/atomic.w"

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
#line 118 "../../../doc/atomic.w"

/*5:*/
#line 131 "../../../doc/atomic.w"

AUGCTX_API int32_t
aug_acq32(volatile int32_t*ptr);

AUGCTX_API void*
aug_acqptr(void*volatile*ptr);

/*:5*//*6:*/
#line 142 "../../../doc/atomic.w"

AUGCTX_API void
aug_rel32(volatile int32_t*ptr,int32_t val);

AUGCTX_API void
aug_relptr(void*volatile*ptr,void*val);

/*:6*//*7:*/
#line 155 "../../../doc/atomic.w"

AUGCTX_API aug_bool_t
aug_cas32(volatile int32_t*ptr,int32_t oldval,int32_t newval);

AUGCTX_API aug_bool_t
aug_casptr(void*volatile*ptr,void*oldval,void*newval);

/*:7*//*8:*/
#line 165 "../../../doc/atomic.w"

AUGCTX_API int32_t
aug_tas32(volatile int32_t*ptr,int32_t val);

AUGCTX_API void*
aug_tasptr(void*volatile*ptr,void*val);

/*:8*//*9:*/
#line 175 "../../../doc/atomic.w"

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t delta);

/*:9*/
#line 119 "../../../doc/atomic.w"

#endif

/*:3*/
#line 107 "../../../doc/atomic.w"

/*13:*/
#line 238 "../../../doc/atomic.w"

#define AUGCTX_BUILD
#ifndef AUGCTX_ATOMIC_H
# include "atomic.h"
#endif
/*14:*/
#line 259 "../../../doc/atomic.w"

AUGCTX_API int32_t
aug_acq32(volatile int32_t*ptr)
{
int32_t val= *ptr;
aug_rmb();
return val;
}

AUGCTX_API void*
aug_acqptr(void*volatile*ptr)
{
void*val= *ptr;
aug_rmb();
return val;
}

/*:14*//*15:*/
#line 278 "../../../doc/atomic.w"

AUGCTX_API void
aug_rel32(volatile int32_t*ptr,int32_t val)
{
aug_wmb();
*ptr= val;
}

AUGCTX_API void
aug_relptr(void*volatile*ptr,void*val)
{
aug_wmb();
*ptr= val;
}

/*:15*/
#line 243 "../../../doc/atomic.w"

#if defined(__APPLE__) && defined(__MACH__)
/*16:*/
#line 296 "../../../doc/atomic.w"

AUGCTX_API aug_bool_t
aug_cas32(volatile int32_t*ptr,int32_t oldval,int32_t newval)
{
return OSAtomicCompareAndSwap32Barrier(oldval,newval,ptr);
}

AUGCTX_API aug_bool_t
aug_casptr(void*volatile*ptr,void*oldval,void*newval)
{
return OSAtomicCompareAndSwapPtrBarrier(oldval,newval,ptr);
}

AUGCTX_API int32_t
aug_tas32(volatile int32_t*ptr,int32_t val)
{
int32_t oldval;
do{
oldval= *ptr;
}while(!OSAtomicCompareAndSwap32Barrier(oldval,val,ptr));
return oldval;
}

AUGCTX_API void*
aug_tasptr(void*volatile*ptr,void*val)
{
void*oldval;
do{
oldval= *ptr;
}while(!OSAtomicCompareAndSwapPtrBarrier(oldval,val,ptr));
return oldval;
}

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t delta)
{
return OSAtomicAdd32Barrier(delta,ptr)-delta;
}

/*:16*/
#line 245 "../../../doc/atomic.w"

#elif defined(__GNUC__) && (__GNUC__ >  4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
/*17:*/
#line 342 "../../../doc/atomic.w"

AUGCTX_API aug_bool_t
aug_cas32(volatile int32_t*ptr,int32_t oldval,int32_t newval)
{
return __sync_bool_compare_and_swap(ptr,oldval,newval);
}

AUGCTX_API aug_bool_t
aug_casptr(void*volatile*ptr,void*oldval,void*newval)
{
return __sync_bool_compare_and_swap(ptr,oldval,newval);
}

AUGCTX_API int32_t
aug_tas32(volatile int32_t*ptr,int32_t val)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__) \
    || defined(__x86_64__) || defined(__amd64__)
return __sync_lock_test_and_set(ptr,val);
#else
int32_t oldval;
do{
oldval= *ptr;
}while(!__sync_bool_compare_and_swap(ptr,oldval,val));
return oldval;
#endif
}

AUGCTX_API void*
aug_tasptr(void*volatile*ptr,void*val)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__) \
    || defined(__x86_64__) || defined(__amd64__)
return __sync_lock_test_and_set(ptr,val);
#else
void*oldval;
do{
oldval= *ptr;
}while(!__sync_bool_compare_and_swap(ptr,oldval,val));
return oldval;
#endif
}

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t delta)
{
return __sync_fetch_and_add(ptr,delta);
}

/*:17*/
#line 247 "../../../doc/atomic.w"

#elif defined(__GNUC__) && (defined(__i486__) || defined(__i586__) \
 || defined(__i686__) || defined(__x86_64__) || defined(__amd64__))
/*18:*/
#line 394 "../../../doc/atomic.w"

AUGCTX_API aug_bool_t
aug_cas32(volatile int32_t*ptr,int32_t oldval,int32_t newval)
{
/*23:*/
#line 535 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"cmpxchgl %3, %1\n\t"
"sete %%al\n\t"
"andl $0xff, %0"
:"=a"(oldval),"=m"(*ptr)
:"0"(oldval),"q"(newval)
:"memory","cc"
);

/*:23*/
#line 398 "../../../doc/atomic.w"

return oldval;
}

AUGCTX_API aug_bool_t
aug_casptr(void*volatile*ptr,void*oldval,void*newval)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__)
/*23:*/
#line 535 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"cmpxchgl %3, %1\n\t"
"sete %%al\n\t"
"andl $0xff, %0"
:"=a"(oldval),"=m"(*ptr)
:"0"(oldval),"q"(newval)
:"memory","cc"
);

/*:23*/
#line 406 "../../../doc/atomic.w"

#else
/*24:*/
#line 548 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"cmpxchgq %3, %1\n\t"
"sete %%al\n\t"
"andl $0xff, %0"
:"=a"(oldval),"=m"(*ptr)
:"0"(oldval),"q"(newval)
:"memory","cc"
);

/*:24*/
#line 408 "../../../doc/atomic.w"

#endif
return(aug_bool_t)oldval;
}

AUGCTX_API int32_t
aug_tas32(volatile int32_t*ptr,int32_t val)
{
/*25:*/
#line 563 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xchgl %2, %1\n\t"
:"=a"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:25*/
#line 416 "../../../doc/atomic.w"

return val;
}

AUGCTX_API void*
aug_tasptr(void*volatile*ptr,void*val)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__)
/*25:*/
#line 563 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xchgl %2, %1\n\t"
:"=a"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:25*/
#line 424 "../../../doc/atomic.w"

#else
/*26:*/
#line 574 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xchgq %2, %1\n\t"
:"=a"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:26*/
#line 426 "../../../doc/atomic.w"

#endif
return val;
}

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t val)
{
/*27:*/
#line 588 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xaddl %2, %1"
:"=q"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:27*/
#line 434 "../../../doc/atomic.w"

return val;
}

/*:18*/
#line 250 "../../../doc/atomic.w"

#elif defined(_WIN32)
/*19:*/
#line 447 "../../../doc/atomic.w"

#include <windows.h>

AUGCTX_API aug_bool_t
aug_cas32(volatile int32_t*ptr,int32_t oldval,int32_t newval)
{
return oldval==InterlockedCompareExchange((volatile long*)ptr,newval,oldval);
}

AUGCTX_API aug_bool_t
aug_casptr(void*volatile*ptr,void*oldval,void*newval)
{
return oldval==InterlockedCompareExchangePointer(ptr,newval,oldval);
}

AUGCTX_API int32_t
aug_tas32(volatile int32_t*ptr,int32_t val)
{
return InterlockedExchange((volatile long*)ptr,val);
}

AUGCTX_API void*
aug_tasptr(void*volatile*ptr,void*val)
{
/*20:*/
#line 484 "../../../doc/atomic.w"

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4311  4312)
#endif

/*:20*/
#line 471 "../../../doc/atomic.w"

return InterlockedExchangePointer((volatile long*)ptr,val);
/*21:*/
#line 492 "../../../doc/atomic.w"

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

/*:21*/
#line 473 "../../../doc/atomic.w"

}

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t delta)
{
return InterlockedExchangeAdd((volatile long*)ptr,delta);
}

/*:19*/
#line 252 "../../../doc/atomic.w"

#else
# error No implementation
#endif

/*:13*/
#line 108 "../../../doc/atomic.w"

/*10:*/
#line 182 "../../../doc/atomic.w"

#include <stdio.h>
#include <stdlib.h>
/*11:*/
#line 190 "../../../doc/atomic.w"

#define die_(file, line, what) \
(fprintf(stderr, "%s:%d: %s\n", file, line, what), fflush(NULL), exit(1))

#define die(what) \
die_(__FILE__, __LINE__, what)

#define check(expr) \
(expr) ? (void)0 : die("check [" #expr "] failed.")

/*:11*/
#line 185 "../../../doc/atomic.w"

/*12:*/
#line 202 "../../../doc/atomic.w"

int
main(int argc,char*argv[])
{
int32_t i;

aug_rel32(&i,100);
check(100==aug_acq32(&i));

check(aug_cas32(&i,100,101));
check(101==i);
check(!aug_cas32(&i,100,101));
check(101==i);

aug_rel32(&i,100);
check(100==aug_tas32(&i,101));
check(101==i);
check(101==aug_tas32(&i,101));
check(101==i);

aug_rel32(&i,0);
check(0==aug_add32(&i,1));
check(1==i);
check(1==aug_add32(&i,-1));
check(0==i);

aug_rmb();
aug_wmb();
aug_mb();
return 0;
}

/*:12*/
#line 186 "../../../doc/atomic.w"


/*:10*/
#line 109 "../../../doc/atomic.w"


/*:2*/
