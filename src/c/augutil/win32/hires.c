/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"
#include "augsys/windows.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_hires_ {
    LARGE_INTEGER freq_, start_;
    aug_ctx* ctx_;
};

AUGUTIL_API aug_hires_t
aug_createhires(aug_ctx* ctx)
{
    struct aug_hires local;
    aug_hires_t hires;
    assert(ctx);

    if (!QueryPerformanceFrequency(&local.freq_)
        || !QueryPerformanceCounter(&local.start_)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return NULL;
    }

    {
        aug_mpool* mpool = aug_getmpool(ctx);
        hires = aug_malloc(mpool, sizeof(struct aug_hires_));
        aug_release(mpool);
    }

    aug_retain(ctx);
    local.ctx_ = ctx;

    memcpy(hires, &local, sizeof(*hires));
    return hires;
}

AUGUTIL_API aug_result
aug_destroyhires(aug_hires_t hires)
{
    aug_ctx* ctx;
    assert(hires);

    ctx = hires->ctx_;

    {
        aug_mpool* mpool = aug_getmpool(ctx);
        aug_free(mpool, hires);
        aug_release(mpool);
    }

    aug_release(ctx);

    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_resethires(aug_hires_t hires)
{
    if (!QueryPerformanceCounter(&hires->start_)) {
        aug_setwin32errinfo(aug_geterrinfo(hires->ctx_), __FILE__, __LINE__,
                            GetLastError());
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGUTIL_API double*
aug_elapsed(aug_hires_t hires, double* sec)
{
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) {
        aug_setwin32errinfo(aug_geterrinfo(hires->ctx_), __FILE__, __LINE__,
                            GetLastError());
        return NULL;
    }

    /* Ticks relative to start. */

    now.QuadPart -= hires->start_.QuadPart;
    *sec = (double)now.QuadPart / (double)hires->freq_.QuadPart;
    return sec;
}
