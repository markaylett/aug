% Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett \.{<}mark.aylett\.{@@}gmail.com\.{>}

% This file is part of Aug written by Mark Aylett.

% Aug is released under the GPL with the additional exemption that compiling,
% linking, and/or using OpenSSL is allowed.

% Aug is free software; you can redistribute it and/or modify it under the
% terms of the GNU General Public License as published by the Free Software
% Foundation; either version 2 of the License, or (at your option) any later
% version.

% Aug is distributed in the hope that it will be useful, but WITHOUT ANY
% WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
% FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
% details.

% You should have received a copy of the GNU General Public License along with
% this program; if not, write to the Free Software Foundation, Inc., 51
% Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

\datethis

\def\title{TAMING ATOMICS (Version 1.0)}
\def\marayl{\pdfURL{Mark Aylett}{mailto:mark.aylett@@gmail.com}}

\def\AUG/{{\sc AUG}}
\def\GCC/{{\sc GCC}}
\def\GNU/{{\sc GNU}}
\def\Java/{{\sc Java}}
\def\LINUX/{{\sc LINUX}}
\def\MSVC/{{\sc MSVC}}
\def\OSX/{{\sc OSX}}
\def\WINDOWS/{{\sc WINDOWS}}
\def\x86/{{\sc x86}}

\centerline{\titlefont Taming Atomics}
\vskip 15pt

\centerline{(Version 1.0)}
\vskip 15pt

Copyright \copyright\ 2004, 2005, 2006, 2007, 2008, 2009 \marayl
\bigskip\noindent

This file is part of \AUG/ written by \marayl.
\smallskip\noindent

\AUG/ is released under the GPL with the additional exemption that compiling,
linking, and/or using OpenSSL is allowed.
\smallskip\noindent

\AUG/ is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.
\smallskip\noindent

\AUG/ is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
details.
\smallskip\noindent

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51
Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

\pageno=\contentspagenumber \advance\pageno by 1

@s AUG_API extern
@s AUG_BUILD extern
@s AUG_EXTERNC extern

@s int32_t int
@s int64_t int
@s aug_bool_t int

@f line normal

@* Introduction. Atomic operations are the building blocks of lock-free
algorithms. An \\{atomic} operation is one that can be completed atomically
with respect to all other program operations. A portable set of these
operations are described in this document.

Memory barriers are closely related to atomic operations. Memory barriers
control the compiler's, and, if necessary, the processor's ability to re-order
memory accesses.

Read barriers force pending reads to complete, and prevent subsequent reads
from being re-ordered before the barrier. Read barriers are used to implement
\\{read-acquire} semantics.

Write barriers force pending writes to complete, and prevent subsequent writes
from being re-ordered before the barrier. Write barriers are used to implement
\\{write-release} semantics.

Read and write barriers are `half barriers', because they prevent re-ordering
of either reads or writes. A full barrier combines the effects of both a read
and a write barrier.

@ All operations are implemented in a single source and accompanying header
file, which makes a trivial addition to any existing project.

@c
@(atomic.h@>@/
@(atomic.c@>@/
@<test@>@/

@ The \.{atomic.h} header file provides the memory barrier macros, and forward
declarations for the atomic functions.

@(atomic.h@>=
#ifndef AUG_ATOMIC_H
#define AUG_ATOMIC_H
@<preamble@>@/
@<memory barriers@>@/
@<declarations@>@/
#endif /* |AUG_ATOMIC_H| */

@* Function Declarations. The \\{volatile} qualifier suppresses certain
compiler optimizations.  In particular, it prevents a compiler from either
caching memory contents in registers, or removing redundant memory accesses.
Atomic function signatures often include a volatile pointer parameter.

@ The |aug_acq32()| and |aug_acqptr()| have \\{read-acquire} semantics.  This
is equivalent to a volatile read in \Java/, which happens before any read that
occurs later in the program order.

@<decl...@>+=
AUG_API int32_t
aug_acq32(volatile int32_t* ptr);@/

AUG_API void*
aug_acqptr(void* volatile* ptr);

@ The |aug_rel32()| and |aug_relptr()| functions have \\{write-release}
semantics. This is equivalent to a volatile write in \Java/, which happens
after any write that occurs earlier in the program order.

@<decl...@>+=
AUG_API void
aug_rel32(volatile int32_t* ptr, int32_t val);@/

AUG_API void
aug_relptr(void* volatile* ptr, void* val);

@ The \\{compare-and-set} functions assign |newval| to |*ptr| if, and only if,
|oldval| equals |*ptr|.  The result of the comparison is returned.  This
differs from \\{compare-and-swap} functions, which return the previous value.
These functions generate a full memory barrier -- meaning no memory references
will be re-ordered across the operation.

@<decl...@>+=
AUG_API aug_bool_t
aug_cas32(volatile int32_t* ptr, int32_t oldval, int32_t newval);@/

AUG_API aug_bool_t
aug_casptr(void* volatile* ptr, void* oldval, void* newval);

@ The \\{test-and-swap} functions assign |val| to |*ptr|, and return the
previous value. These functions generate a read memory barrier.

@<decl...@>+=
AUG_API int32_t
aug_tas32(volatile int32_t* ptr, int32_t val);@/

AUG_API void*
aug_tasptr(void* volatile* ptr, void* val);

@ The |aug_add32()| function increments |*ptr| by |delta|, and returns the
previous value.  This function generates a full memory barrier.

@<decl...@>+=
AUG_API int32_t
aug_add32(volatile int32_t* ptr, int32_t delta);

@* Test Program. The test program serves to illustrate several common
use-cases.

@<test@>=
#include <stdio.h>
#include <stdlib.h>
@<check macros@>@/
@<main@>@/

@ The |check()| macro verifies test results.

@<check...@>=
#define die_(file, line, what) \
(fprintf(stderr, "%s:%d: %s\n", file, line, what), fflush(NULL), exit(1))

#define die(what) \
die_(__FILE__, __LINE__, what)

#define check(expr) \
(expr) ? (void)0 : die("check [" #expr "] failed.")

@ The test cases are executed within the |main| function.

@<main@>=
int
main(int argc, char* argv[])
{
    int32_t i;

    aug_rel32(&i, 100);
    check(100 == aug_acq32(&i));

    check(aug_cas32(&i, 100, 101));
    check(101 == i);
    check(!aug_cas32(&i, 100, 101));
    check(101 == i);

    aug_rel32(&i, 100);
    check(100 == aug_tas32(&i, 101));
    check(101 == i);
    check(101 == aug_tas32(&i, 101));
    check(101 == i);

    aug_rel32(&i, 0);
    check(0 == aug_add32(&i, 1));
    check(1 == i);
    check(1 == aug_add32(&i, -1));
    check(0 == i);

    aug_rmb();
    aug_wmb();
    aug_mb();
    return 0;
}

@* Function Definitions. Where possible, builtin compiler or system functions
are used in preference to hand-crafted assembly.  Assembly is provided for the
\x86/ architecture, as a last resort, where there is no suitable alternative.

@<atomic.c@>=
#define AUG_BUILD
#ifndef AUG_ATOMIC_H
# include "atomic.h"
#endif /* |AUG_ATOMIC_H| */
@<general definitions@>@/
#if defined(__APPLE__) && defined(__MACH__)
@<osx definitions@>@/
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
@<gcc41 definitions@>@/
#elif defined(__GNUC__) && (defined(__i486__) || defined(__i586__) \
 || defined(__i686__) || defined(__x86_64__) || defined(__amd64__))
@<gcc x86 definitions@>@/
#elif defined(_WIN32)
@<win32 definitions@>@/
#else
# error No implementation
#endif

@ The read barrier ensures that |*ptr| is read before all subsequent reads.

@<general...@>+=
AUG_API int32_t
aug_acq32(volatile int32_t* ptr)
{
    int32_t val = *ptr;
    aug_rmb();
    return val;
}

AUG_API void*
aug_acqptr(void* volatile* ptr)
{
    void* val = *ptr;
    aug_rmb();
    return val;
}

@ The write barrier ensures that |*ptr| is written after all previous writes.

@<general...@>+=
AUG_API void
aug_rel32(volatile int32_t* ptr, int32_t val)
{
    aug_wmb();
    *ptr = val;
}

AUG_API void
aug_relptr(void* volatile* ptr, void* val)
{
    aug_wmb();
    *ptr = val;
}

@ The \OSX/ platform does not offer a \\{test-and-swap} operation, so it is
simulated using a \\{compare-and-test}.

@<osx def...@>=
AUG_API aug_bool_t
aug_cas32(volatile int32_t* ptr, int32_t oldval, int32_t newval)
{
    return OSAtomicCompareAndSwap32Barrier(oldval, newval, ptr);
}

AUG_API aug_bool_t
aug_casptr(void* volatile* ptr, void* oldval, void* newval)
{
    return OSAtomicCompareAndSwapPtrBarrier(oldval, newval, ptr);
}

AUG_API int32_t
aug_tas32(volatile int32_t* ptr, int32_t val)
{
    int32_t oldval;
    do {
        oldval = *ptr;
    } while (!OSAtomicCompareAndSwap32Barrier(oldval, val, ptr));
    return oldval;
}

AUG_API void*
aug_tasptr(void* volatile* ptr, void* val)
{
    void* oldval;
    do {
        oldval = *ptr;
    } while (!OSAtomicCompareAndSwapPtrBarrier(oldval, val, ptr));
    return oldval;
}

AUG_API int32_t
aug_add32(volatile int32_t* ptr, int32_t delta)
{
    return OSAtomicAdd32Barrier(delta, ptr) - delta;
}

@ The \GCC/ documentation on builtin atomics states that the only valid value
for |__sync_lock_test_and_set()| on some targets is a constant |1|, and that
the exact value actually stored in |*ptr| is implementation defined.

For this reason, \GCC/'s |__sync_lock_test_and_set()| atomic builtin is only
used on the \x86/ platform, which is known to have a complete implementation.

@<gcc41 def...@>=
AUG_API aug_bool_t
aug_cas32(volatile int32_t* ptr, int32_t oldval, int32_t newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

AUG_API aug_bool_t
aug_casptr(void* volatile* ptr, void* oldval, void* newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

AUG_API int32_t
aug_tas32(volatile int32_t* ptr, int32_t val)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__) \
    || defined(__x86_64__) || defined(__amd64__)
    return __sync_lock_test_and_set(ptr, val);
#else
    int32_t oldval;
    do {
        oldval = *ptr;
    } while (!__sync_bool_compare_and_swap(ptr, oldval, val));
    return oldval;
#endif
}

AUG_API void*
aug_tasptr(void* volatile* ptr, void* val)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__) \
    || defined(__x86_64__) || defined(__amd64__)
    return __sync_lock_test_and_set(ptr, val);
#else
    void* oldval;
    do {
        oldval = *ptr;
    } while (!__sync_bool_compare_and_swap(ptr, oldval, val));
    return oldval;
#endif
}

AUG_API int32_t
aug_add32(volatile int32_t* ptr, int32_t delta)
{
	return  __sync_fetch_and_add(ptr, delta);
}

@ Inline assembly is used in the absence of atomic builtins on older versions
of \GCC/.

@<gcc x86 def...@>=
AUG_API aug_bool_t
aug_cas32(volatile int32_t* ptr, int32_t oldval, int32_t newval)
{
    @<cmpxchgl asm@>@;
    return oldval;
}

AUG_API aug_bool_t
aug_casptr(void* volatile* ptr, void* oldval, void* newval)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__)
    @<cmpxchgl asm@>@;
#else /* |__x86_64__ || __amd64__| */
    @<cmpxchgq asm@>@;
#endif /* |__x86_64__ || __amd64__| */
    return (aug_bool_t) oldval;
}

AUG_API int32_t
aug_tas32(volatile int32_t* ptr, int32_t val)
{
    @<xchgl asm@>@;
    return val;
}

AUG_API void*
aug_tasptr(void* volatile* ptr, void* val)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__)
    @<xchgl asm@>@;
#else /* |__x86_64__ || __amd64__| */
    @<xchgq asm@>@;
#endif /* |__x86_64__ || __amd64__| */
    return val;
}

AUG_API int32_t
aug_add32(volatile int32_t* ptr, int32_t val)
{
    @<xaddl asm@>@;
    return val;
}

@ The Windows header is required for interlocked functions that are
unavailable as intrinsics.

|_InterlockedCompareExchangePointer()| is one such example.

|InterlockedExchangePointer()| may be a macro that delegates to
|InterlockedExchange()|. This involves casting pointers to integers, which
generates warnings. These warnings are disabled during the call.

@<win32 def...@>=
#include <windows.h>

AUG_API aug_bool_t
aug_cas32(volatile int32_t* ptr, int32_t oldval, int32_t newval)
{
	return oldval == InterlockedCompareExchange((volatile long*) ptr, newval, oldval);
}

AUG_API aug_bool_t
aug_casptr(void* volatile* ptr, void* oldval, void* newval)
{
	return oldval == InterlockedCompareExchangePointer(ptr, newval, oldval);
}

AUG_API int32_t
aug_tas32(volatile int32_t* ptr, int32_t val)
{
	return InterlockedExchange((volatile long*) ptr, val);
}

AUG_API void*
aug_tasptr(void* volatile* ptr, void* val)
{
    @<disable warnings@>@;
	return InterlockedExchangePointer((volatile long*) ptr, val);
    @<enable warnings@>@;
}

AUG_API int32_t
aug_add32(volatile int32_t* ptr, int32_t delta)
{
	return InterlockedExchangeAdd((volatile long*) ptr, delta);
}

@ Disable pointer to integer cast warnings.

@<disable...@>=
#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4311 @, 4312)
#endif /* |_MSC_VER| */

@ Enable pointer to integer cast warnings.

@<enable...@>=
#if defined(_MSC_VER)
# pragma warning(pop)
#endif /* |_MSC_VER| */

@* GCC Inline Assembly. Inline assembly blocks are introduced with the
|__asm__| keyword. Assembly blocks have four sections: instructions, output
operands, input operands and the clobber list. Output and input operands are
specified using constraints.  The |__volatile__| qualifier indicates that the
instructions have important side-effects; \GCC/ will not delete volatile
assembly if it is reachable.

Example output operands are: |"=a"|, |"=m"| and |"=q"|.  Where |=| indicates
an output operand, |a| is the |eax| register, |m| is a memory operand, and |q|
selects one of the |eax|, |ebx|, |ecx| or |edx| registers.  These are the
constraints.

The |a|, |m| and |q| constraints can also be used as input constraints. A |0|
constraint specifies the same constraint as the 0th output operand. If the 0th
output constraint is |a| (|eax|), for example, then a |0| input constraint
means that |eax| is also an input operand.

When |memory| appears in the clobber list, it avoids having memory values
cached in registers, and prevents load and store optimizations.  This should
be used whenever instructions access memory in an unpredictable way.

A |cc| in the clobber list means that the instructions can alter the condition
code register.

@ The |cmpxchg| instruction compares the |a| operand with the |m| output
operand. If the two values are equal, then |zf| is set and the |q| input
operand is loaded into |m|. Otherwise, |zf| is cleared and |m| is loaded into
|a|.

This instruction can be used with a |lock| prefix. The |lock| prefix ensures
that a read-modify-write operation on memory is carried out atomically.

The |sete| instruction sets the byte in the |a| operand to |1| if |zf| is set.
Otherwise, it sets |a| to |0|.

The final instruction applies a mask to clear |a|'s remaining bytes, leaving
the boolean return value in |a|.

@<cmpxchgl asm@>=
__asm__ __volatile__(@/
"lock\n\t"@/
"cmpxchgl %3, %1\n\t"@/
"sete %%al\n\t"@/
"andl $0xff, %0"@/
: "=a" (oldval), "=m" (*ptr)@/
: "0" (oldval), "q" (newval)@/
: "memory", "cc"@/
);@/

@ The equivalent on 64 bit architectures.

@<cmpxchgq asm@>=
__asm__ __volatile__(@/
"lock\n\t"@/
"cmpxchgq %3, %1\n\t"@/
"sete %%al\n\t"@/
"andl $0xff, %0"@/
: "=a" (oldval), "=m" (*ptr)@/
: "0" (oldval), "q" (newval)@/
: "memory", "cc"@/
);@/

@ The |xchg| instruction swaps the |a| operand with the |m| output operand.
An explicit |lock| prefix is added for clarity, even though it is implied for
this particular instruction due to the memory reference, |m|.

@<xchgl asm@>=
__asm__ __volatile__(@/
"lock\n\t"@/
"xchgl %2, %1\n\t"@/
: "=a" (val), "=m" (*ptr)@/
: "0" (val)@/
: "memory"@/
);@/

@ The equivalent on 64 bit architectures.

@<xchgq asm@>=
__asm__ __volatile__(@/
"lock\n\t"@/
"xchgq %2, %1\n\t"@/
: "=a" (val), "=m" (*ptr)@/
: "0" (val)@/
: "memory"@/
);@/

@ The |xadd| instruction swaps the |a| operand with the |m| output operand, it
then stores the sum of the two operands in |m|, so that |a| effectively
contains the value of |m| prior to the store.  This instruction can be used
with a |lock| prefix.

@<xaddl asm@>=
__asm__ __volatile__(@/
"lock\n\t"@/
"xaddl %2, %1"@/
: "=q" (val), "=m" (*ptr)@/
: "0" (val)@/
: "memory"@/
);@/

@* Memory Barriers. Memory barrier instructions are not required on
Uni-Processor (UP) systems, because the processor orders overlapping accesses
with respect to itself.  The compiler, however, will still need to be
prevented from re-ordering instructions.

@<mem...@>=
#if defined(__APPLE__) && defined(__MACH__)
@<osx barriers@>@/
#elif defined(__GNUC__)
# if !ENABLE_SMP
/* |Prevent compiler re-ordering.| */
#  define aug_mb() __asm__ __volatile__("":::"memory")
# else /* |ENABLE_SMP| */
#  if defined(__SSE2__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__)
@<gcc sse2 i586 barriers@>@/
#  elif defined(__i386__) || defined(__i486__)
@<gcc i386 barriers@>@/
#  elif defined(__ia64__)
@<gcc ia64 barriers@>@/
#  elif defined(__alpha__)
@<gcc alpha barriers@>@/
#  elif defined(__PPC__)
@<gcc ppc barriers@>@/
#  elif defined(__sparc__)
@<gcc sparc barriers@>@/
#  elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
@<gcc41 barriers@>@/
#  else
#   error No implementation
#  endif
# endif /* |ENABLE_SMP| */
#elif defined(_MSC_VER)
@<msc barriers@>@/
#else
# error No implementation
#endif

/* |If not defined, define read barrier in terms of full barrier.| */

#if !defined(aug_rmb)
# define aug_rmb aug_mb
#endif /* |!aug_rmb| */

/* |If not defined, define write barrier in terms of full barrier.| */

#if !defined(aug_wmb)
# define aug_wmb aug_mb
#endif /* |!aug_wmb| */

@ \OSX/ memory barriers.

@<osx bar...@>=
# include <libkern/OSAtomic.h>
# define aug_mb() OSMemoryBarrier()

@ \\{fence} instructions were introduced with the SSE2 instruction set, which
is available on Pentium class machines.

@<gcc sse2 i586 bar...@>=
#   define aug_mb()  __asm__ __volatile__("mfence":::"memory")
#   define aug_rmb() __asm__ __volatile__("lfence":::"memory")
#   define aug_wmb() __asm__ __volatile__("sfence":::"memory")

@ i386 memory barriers.

@<gcc i386 bar...@>=
#   define aug_mb()  __asm__ __volatile__("lock; addl $0,0(%%esp)":::"memory")
#   define aug_wmb() __asm__ __volatile__("":::"memory")

@ Itanium memory barriers.

@<gcc ia64 bar...@>=
#   define aug_mb()  __asm__ __volatile__("mf":::"memory")

@ Alpha memory barriers.

@<gcc alpha bar...@>=
#   define aug_mb()  __asm__ __volatile__("mb":::"memory")
#   define aug_wmb() __asm__ __volatile__("wmb":::"memory")

@ Power PC memory barriers.

@<gcc ppc bar...@>=
#   define aug_mb()  __asm__ __volatile__("sync":::"memory")
#   define aug_wmb() __asm__ __volatile__("eieio":::"memory")

@ Sparc memory barriers.

@<gcc sparc bar...@>=
#   define aug_mb()  __asm__ __volatile__(
"membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad":::"memory")
#   define aug_rmb() __asm__ __volatile__("membar #LoadLoad":::"memory")
#   define aug_wmb() __asm__ __volatile__("membar #StoreStore":::"memory")

@ \GCC/ builtin memory barriers.

@<gcc41 bar...@>=
#   define aug_mb()  __sync_synchronize()

@ Read barriers are only available on some MSC versions.

@<msc bar...@>=
# define aug_mb()  _ReadWriteBarrier()
# if _MSC_VER >= 1400
#  define aug_rmb() _ReadBarrier()
# endif /* |_MSC_VER >= 1400| */
# define aug_wmb() _WriteBarrier()

@* Header Preamble. Configuration, linkage and intrinsics.

@<pream...@>=
@<options@>@/
@<linkage@>@/
#if defined(_MSC_VER)
@<msc intrinsics@>@/
#endif /* |_MSC_VER| */
@<standard integer types@>@/

@ When SMP is disabled, the memory barrier macros will simply prevent compiler
re-ordering.

@<options@>=
#if !defined(ENABLE_SMP)
# define ENABLE_SMP 1
#endif /* |!ENABLE_SMP| */

#if !defined(AUG_SHARED)
# if defined(DLL_EXPORT) || defined(_WINDLL)
#  define AUG_SHARED
# endif /* |DLL_EXPORT || _WINDLL| */
#endif /* |!AUG_SHARED| */

@ When building a shared library, |AUG_SHARED| should be defined to ensure the
creation of a dynamic symbol table.

@<linkage@>=
#if !defined(__cplusplus)
# define AUG_EXTERNC extern
#else /* |__cplusplus| */
# define AUG_EXTERNC extern "C"
#endif /* |__cplusplus| */

#if defined(__CYGWIN__) || defined(__MINGW32__)
# define AUG_EXPORT __attribute__ ((dllexport))
# define AUG_IMPORT __attribute__ ((dllimport))
#elif defined(_MSC_VER)
# define AUG_EXPORT __declspec(dllexport)
# define AUG_IMPORT __declspec(dllimport)
#else /* |!__CYGWIN__ && !__MINGW__ && !__MSC_VER| */
# define AUG_EXPORT
# define AUG_IMPORT
#endif /* |!__CYGWIN__ && !__MINGW__ && !__MSC_VER| */

#if !defined(AUG_SHARED)
# define AUG_API AUG_EXTERNC
#else /* |AUG_SHARED| */
# if !defined(AUG_BUILD)
#  define AUG_API AUG_EXTERNC AUG_IMPORT
# else /* |AUG_BUILD| */
#  define AUG_API AUG_EXTERNC AUG_EXPORT
# endif /* |AUG_BUILD| */
#endif /* |AUG_SHARED| */

@ Note that |_InterlockedCompareExchangePointer()| is only available on 64 bit
architectures, and |_ReadBarrier()| for 64 bit compilers. See \.{intrin.h} for
details.

@<msc intrin...@>=
# if _MSC_VER >= 1400
#  include <intrin.h>
#  pragma intrinsic(_ReadBarrier)
# else /* |_MSC_VER < 1400| */
AUG_EXTERNC void _WriteBarrier(void);
AUG_EXTERNC void _ReadWriteBarrier(void);
AUG_EXTERNC long _InterlockedCompareExchange (long volatile *, long, long);
AUG_EXTERNC long _InterlockedExchange (long volatile *, long);
AUG_EXTERNC long _InterlockedExchangeAdd(long volatile *, long);
# endif /* |_MSC_VER < 1400| */

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

@ Type aliases for |int32_t| and |int64_t| are required when compiling under
MSC.

@<standard integer types@>=
typedef int aug_bool_t;
#if !defined(_MSC_VER)
# include <stdint.h>
#else /* |_MSC_VER| */
typedef __int32 int32_t;
typedef __int64 int64_t;
#endif /* |_MSC_VER| */

@* Index.
