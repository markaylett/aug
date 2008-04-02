/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"
#include "augsys/time.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_hires_ {
    struct timeval start_;
    aug_ctx* ctx_;
};

AUGUTIL_API aug_hires_t
aug_createhires(aug_ctx* ctx)
{
    struct aug_hires local;
    aug_hires_t hires;
    assert(ctx);

    if (-1 == aug_gettimeofday(&local.start_, NULL)) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
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

AUGUTIL_API int
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
    if (-1 == aug_gettimeofday(&hires->start_, NULL)) {
        aug_setposixerrinfo(aug_geterrinfo(hires->ctx_), __FILE__, __LINE__,
                            errno);
        return AUG_FAILURE;
    }
    return AUG_SUCCESS;
}

AUGUTIL_API double*
aug_elapsed(aug_hires_t hires, double* sec)
{
    struct timeval now;
    if (-1 == aug_gettimeofday(&now, NULL)) {
        aug_setposixerrinfo(aug_geterrinfo(hires->ctx_), __FILE__, __LINE__,
                            errno);
        return NULL;
    }
    aug_tvsub(&now, &hires->start_);
    *sec = (double)now.tv_sec + ((double)now.tv_usec / 1000000.0);
    return sec;
}
