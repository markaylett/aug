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
/*13:*/
#line 237 "../../../doc/atomic.w"

#define AUGCTX_BUILD
#ifndef AUGCTX_ATOMIC_H
# include "atomic.h"
#endif
/*14:*/
#line 258 "../../../doc/atomic.w"

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
#line 277 "../../../doc/atomic.w"

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
#line 242 "../../../doc/atomic.w"

#if defined(__APPLE__) && defined(__MACH__)
/*16:*/
#line 295 "../../../doc/atomic.w"

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
#line 244 "../../../doc/atomic.w"

#elif defined(__GNUC__) && (__GNUC__ >  4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
/*17:*/
#line 341 "../../../doc/atomic.w"

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
#line 246 "../../../doc/atomic.w"

#elif defined(__GNUC__) && (defined(__i486__) || defined(__i586__) \
 || defined(__i686__) || defined(__x86_64__) || defined(__amd64__))
/*18:*/
#line 393 "../../../doc/atomic.w"

AUGCTX_API aug_bool_t
aug_cas32(volatile int32_t*ptr,int32_t oldval,int32_t newval)
{
/*23:*/
#line 534 "../../../doc/atomic.w"

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
#line 397 "../../../doc/atomic.w"

return oldval;
}

AUGCTX_API aug_bool_t
aug_casptr(void*volatile*ptr,void*oldval,void*newval)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__)
/*23:*/
#line 534 "../../../doc/atomic.w"

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
#line 405 "../../../doc/atomic.w"

#else
/*24:*/
#line 547 "../../../doc/atomic.w"

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
#line 407 "../../../doc/atomic.w"

#endif
return(aug_bool_t)oldval;
}

AUGCTX_API int32_t
aug_tas32(volatile int32_t*ptr,int32_t val)
{
/*25:*/
#line 562 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xchgl %2, %1\n\t"
:"=a"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:25*/
#line 415 "../../../doc/atomic.w"

return val;
}

AUGCTX_API void*
aug_tasptr(void*volatile*ptr,void*val)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__)
/*25:*/
#line 562 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xchgl %2, %1\n\t"
:"=a"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:25*/
#line 423 "../../../doc/atomic.w"

#else
/*26:*/
#line 573 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xchgq %2, %1\n\t"
:"=a"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:26*/
#line 425 "../../../doc/atomic.w"

#endif
return val;
}

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t val)
{
/*27:*/
#line 587 "../../../doc/atomic.w"

__asm__ __volatile__(
"lock\n\t"
"xaddl %2, %1"
:"=q"(val),"=m"(*ptr)
:"0"(val)
:"memory"
);

/*:27*/
#line 433 "../../../doc/atomic.w"

return val;
}

/*:18*/
#line 249 "../../../doc/atomic.w"

#elif defined(_WIN32)
/*19:*/
#line 446 "../../../doc/atomic.w"

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
#line 483 "../../../doc/atomic.w"

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4311  4312)
#endif

/*:20*/
#line 470 "../../../doc/atomic.w"

return InterlockedExchangePointer((volatile long*)ptr,val);
/*21:*/
#line 491 "../../../doc/atomic.w"

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

/*:21*/
#line 472 "../../../doc/atomic.w"

}

AUGCTX_API int32_t
aug_add32(volatile int32_t*ptr,int32_t delta)
{
return InterlockedExchangeAdd((volatile long*)ptr,delta);
}

/*:19*/
#line 251 "../../../doc/atomic.w"

#else
# error No implementation
#endif

/*:13*/
