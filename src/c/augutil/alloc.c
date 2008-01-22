/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/alloc.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/lock.h"

#include <stdlib.h>
#include <string.h>

enum {
    ALLOC32_,
    ALLOC64_,
    MALLOC_
};

static struct aug_allocs32 free32_ = AUG_HEAD_INITIALIZER(free32_);
AUG_ALLOCATOR(allocate32_, &free32_, aug_alloc32, 64)

static struct aug_allocs64 free64_ = AUG_HEAD_INITIALIZER(free64_);
AUG_ALLOCATOR(allocate64_, &free64_, aug_alloc64, 64)

static void
freemem32_(void* ptr)
{
    char* mem = ptr;
    struct aug_alloc32* alloc = (struct aug_alloc32*)
        (mem - offsetof(struct aug_alloc32, mem_));
    aug_destroyalloc32(alloc);
}

static void
freemem64_(void* ptr)
{
    char* mem = ptr;
    struct aug_alloc64* alloc = (struct aug_alloc64*)
        (mem - offsetof(struct aug_alloc64, mem_));
    aug_destroyalloc64(alloc);
}

AUGUTIL_API void
aug_destroyalloc32s(struct aug_allocs32* allocs32)
{
    if (!AUG_EMPTY(allocs32)) {
        aug_lock();
        AUG_CONCAT(&free32_, allocs32);
        aug_unlock();
    }
}

AUGUTIL_API void
aug_destroyalloc32(struct aug_alloc32* alloc32)
{
    aug_lock();
    AUG_INSERT_TAIL(&free32_, alloc32);
    aug_unlock();
}

AUGUTIL_API struct aug_alloc32*
aug_createalloc32(void)
{
    struct aug_alloc32* alloc32;

    aug_lock();
    alloc32 = allocate32_();
    aug_unlock();

    return alloc32;
}

AUGUTIL_API void
aug_destroyalloc64s(struct aug_allocs64* allocs64)
{
    if (!AUG_EMPTY(allocs64)) {
        aug_lock();
        AUG_CONCAT(&free64_, allocs64);
        aug_unlock();
    }
}

AUGUTIL_API void
aug_destroyalloc64(struct aug_alloc64* alloc64)
{
    aug_lock();
    AUG_INSERT_TAIL(&free64_, alloc64);
    aug_unlock();
}

AUGUTIL_API struct aug_alloc64*
aug_createalloc64(void)
{
    struct aug_alloc64* alloc64;

    aug_lock();
    alloc64 = allocate64_();
    aug_unlock();

    return alloc64;
}

AUGUTIL_API void
aug_freesmall(void* ptr)
{
    static void (*free_[3])(void*) = {
        freemem32_,
        freemem64_,
        free
    };
    char* mem = (char*)ptr - 1;
    free_[(size_t)mem[0]](mem);
}

AUGUTIL_API void*
aug_allocsmall(size_t size)
{
    char* mem = NULL;
    if (size <= 32 - 1) {
        struct aug_alloc32* alloc32 = aug_createalloc32();
        if (alloc32) {
            mem = alloc32->mem_;
            *mem++ = ALLOC32_;
        }
    } else if (size <= 64 - 1) {
        struct aug_alloc64* alloc64 = aug_createalloc64();
        if (alloc64) {
            mem = alloc64->mem_;
            *mem++ = ALLOC64_;
        }
    } else {
        if ((mem = malloc(size + sizeof(void (*)(void*)))))
            *mem++ = MALLOC_;
    }
    return mem;
}
