/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/time.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_hires_ {
    aug_mpool* mpool_;
    struct timeval start_;
};

AUGUTIL_API aug_hires_t
aug_createhires(aug_mpool* mpool)
{
    aug_hires_t hires = aug_malloc(mpool, sizeof(struct aug_hires_));
    if (!hires)
        return NULL;

    hires->mpool_ = mpool;

    if (-1 == aug_gettimeofday(&local.start_, NULL)) {
        aug_setposixerrinfo(aug_geterrinfo(ctx), __FILE__, __LINE__, errno);
        aug_free(mpool, hires);
        return NULL;
    }

    aug_retain(mpool);
    return hires;
}

AUGUTIL_API int
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
    if (-1 == aug_gettimeofday(&hires->start_, NULL)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGUTIL_API double*
aug_elapsed(aug_hires_t hires, double* sec)
{
    struct timeval now;
    if (-1 == aug_gettimeofday(&now, NULL)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return NULL;
    }
    aug_tvsub(&now, &hires->start_);
    *sec = (double)now.tv_sec + ((double)now.tv_usec / 1000000.0);
    return sec;
}
