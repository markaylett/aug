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
#define AUGCTX_BUILD
#include "augctx/mpool.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#define DEBUG 1
#define ONLY_MSPACES 1
#include "malloc.c"

#include <assert.h>
#include <string.h>

static void*
crtcast_(aug_mpool* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_mpoolid)) {

        /* Singleton instance, so no aug_retain() required here. */

        return obj;
    }
    return NULL;
}

static void
crtretain_(aug_mpool* obj)
{
}

static void
crtrelease_(aug_mpool* obj)
{
}

static void*
crtallocmem_(aug_mpool* obj, size_t size)
{
    return malloc(size);
}

static void
crtfreemem_(aug_mpool* obj, void* ptr)
{
    free(ptr);
}

static void*
crtreallocmem_(aug_mpool* obj, void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void*
crtcallocmem_(aug_mpool* obj, size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static const struct aug_mpoolvtbl crtvtbl_ = {
    crtcast_,
    crtretain_,
    crtrelease_,
    crtallocmem_,
    crtfreemem_,
    crtreallocmem_,
    crtcallocmem_
};

static aug_mpool mpool_ = { &crtvtbl_, NULL };

struct dlimpl_ {
    aug_mpool mpool_;
    int refs_;
    mspace mspace_;
};

static void*
dlcast_(aug_mpool* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_mpoolid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
dlretain_(aug_mpool* obj)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
dlrelease_(aug_mpool* obj)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        mspace msp = impl->mspace_;
        mspace_free(msp, impl);
#if DEBUG
        fprintf(stderr, "mspace before destroy...\n");
        mspace_malloc_stats(msp);
#endif /* DEBUG */
        destroy_mspace(msp);
    }
}

static void*
dlallocmem_(aug_mpool* obj, size_t size)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    return mspace_malloc(impl->mspace_, size);
}

static void
dlfreemem_(aug_mpool* obj, void* ptr)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    mspace_free(impl->mspace_, ptr);
}

static void*
dlreallocmem_(aug_mpool* obj, void* ptr, size_t size)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    return mspace_realloc(impl->mspace_, ptr, size);
}

static void*
dlcallocmem_(aug_mpool* obj, size_t nmemb, size_t size)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    return mspace_calloc(impl->mspace_, nmemb, size);
}

static const struct aug_mpoolvtbl dlvtbl_ = {
    dlcast_,
    dlretain_,
    dlrelease_,
    dlallocmem_,
    dlfreemem_,
    dlreallocmem_,
    dlcallocmem_
};

AUGCTX_API aug_mpool*
aug_getcrtmalloc(void)
{
    return &mpool_;
}

AUGCTX_API aug_mpool*
aug_createdlmalloc(void)
{
    mspace msp;
    struct dlimpl_* impl;

    if (!(msp = create_mspace(0, 0)))
        return NULL;

#if DEBUG
    fprintf(stderr, "mspace after create...\n");
    mspace_malloc_stats(msp);
#endif /* DEBUG */

    if (!(impl = mspace_malloc(msp, sizeof(struct dlimpl_)))) {
        destroy_mspace(msp);
        return NULL;
    }

    impl->mpool_.vtbl_ = &dlvtbl_;
    impl->mpool_.impl_ = NULL;

    impl->refs_ = 1;
    impl->mspace_ = msp;

    return &impl->mpool_;
}
