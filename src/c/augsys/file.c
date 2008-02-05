/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/file.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/utility.h"

#include <assert.h>
#include <string.h>

struct impl_ {
    aug_file file_;
    int refs_;
    aug_ctx* ctx_;
};

static void*
cast_(aug_file* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_fileid)) {
        aug_retain(obj);
        return obj;
    } else if (AUG_EQUALID(id, aug_ctxid)) {
        struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
        aug_retain(impl->ctx_);
        return impl->ctx_;
    }
    return NULL;
}

static void
retain_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_ctx* ctx = impl->ctx_;
        aug_mpool* mpool = aug_getmpool(ctx);
        aug_free(mpool, impl);
        aug_release(mpool);
        aug_release(ctx);
    }
}

static int
close_(aug_file* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, file_, obj);
    aug_seterrinfo(aug_geterrinfo(impl->ctx_), __FILE__, __LINE__,
                   "augctx", 1, "%s", "oops - not really open!");
    return -1;
}

static int
setnonblock_(aug_file* obj, int on)
{
    return 0;
}

static aug_fd
getfd_(aug_file* obj)
{
    return 0;
}

static const struct aug_filevtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    close_,
    setnonblock_,
    getfd_
};

AUGSYS_API aug_file*
aug_createfile(aug_ctx* ctx, const char* path)
{
    struct impl_* impl;
    aug_mpool* mpool;
    assert(ctx);

    mpool = aug_getmpool(ctx);
    impl = aug_malloc(mpool, sizeof(struct impl_));
    aug_release(mpool);

    if (!impl)
        return NULL;

    impl->file_.vtbl_ = &vtbl_;
    impl->file_.impl_ = NULL;
    impl->refs_ = 1;

    aug_retain(ctx);

    impl->ctx_ = ctx;

    return &impl->file_;
}
