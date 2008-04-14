/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>  /* ENOMEM */

struct aug_hires_ {
    LARGE_INTEGER freq_, start_;
    aug_mpool* mpool_;
};

AUGUTIL_API aug_hires_t
aug_createhires(void)
{
    aug_mpool* mpool = aug_getmpool(aug_tlx);
    aug_hires_t hires = aug_malloc(mpool, sizeof(struct aug_hires_));

    if (!hires) {
        aug_release(mpool);
        return NULL;
    }

    if (!QueryPerformanceFrequency(&hires->freq_)
        || !QueryPerformanceCounter(&hires->start_)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        aug_free(mpool, hires);
        aug_release(mpool);
        return NULL;
    }

    return hires;
}

AUGUTIL_API aug_result
aug_destroyhires(aug_hires_t hires)
{
    aug_mpool* mpool = hires->mpool_;
    aug_free(mpool, hires);
    aug_release(mpool);
    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_resethires(aug_hires_t hires)
{
    if (!QueryPerformanceCounter(&hires->start_)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGUTIL_API double*
aug_elapsed(aug_hires_t hires, double* sec)
{
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return NULL;
    }

    /* Ticks relative to start. */

    now.QuadPart -= hires->start_.QuadPart;
    *sec = (double)now.QuadPart / (double)hires->freq_.QuadPart;
    return sec;
}
