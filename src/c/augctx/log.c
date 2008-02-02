/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/log.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/types.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

struct impl_ {
    aug_log log_;
    int refs_;
    aug_mpool* mpool_;
};

static void*
cast_(aug_log* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_logid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_log* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_log* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, log_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static int
vprint_(aug_log* obj, int level, const char* format, va_list args)
{
    char buf[1024];
    FILE* file = level > AUG_LOGWARN ? stdout : stderr;
    int ret;

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = vsnprintf(buf, sizeof(buf), format, args);
    AUG_SNTRUNCF(buf, sizeof(buf), ret);

    if (ret < 0)
        return -1;

    fprintf(file, "%s\n", buf);
    fflush(file);
    return 0;
}

static const struct aug_logvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    vprint_
};

AUGCTX_API aug_log*
aug_createstdlog(aug_mpool* mpool)
{
    struct impl_* impl;
    assert(mpool);

    if (!(impl = aug_malloc(mpool, sizeof(struct impl_))))
        return NULL;

    impl->log_.vtbl_ = &vtbl_;
    impl->log_.impl_ = NULL;
    impl->refs_ = 1;

    aug_retain(mpool);

    impl->mpool_ = mpool;

    return &impl->log_;
}
