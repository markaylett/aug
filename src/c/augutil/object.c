/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <string.h>

struct boxintimpl_ {
    aug_boxint boxint_;
    int refs_;
    aug_mpool* mpool_;
    void (*destroy_)(int);
    int i_;
};

static void*
castboxint_(aug_boxint* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_boxintid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retainboxint_(aug_boxint* ob)
{
    struct boxintimpl_* impl = AUG_PODIMPL(struct boxintimpl_, boxint_, ob);
    ++impl->refs_;
}

static void
releaseboxint_(aug_boxint* ob)
{
    struct boxintimpl_* impl = AUG_PODIMPL(struct boxintimpl_, boxint_, ob);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (impl->destroy_)
            impl->destroy_(impl->i_);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static int
getboxint_(aug_boxint* ob)
{
    struct boxintimpl_* impl = AUG_PODIMPL(struct boxintimpl_, boxint_, ob);
    return impl->i_;
}

static const struct aug_boxintvtbl boxintvtbl_ = {
    castboxint_,
    retainboxint_,
    releaseboxint_,
    getboxint_
};

struct boxptrimpl_ {
    aug_boxptr boxptr_;
    int refs_;
    aug_mpool* mpool_;
    void (*destroy_)(void*);
    void* p_;
};

static void*
castboxptr_(aug_boxptr* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_boxptrid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retainboxptr_(aug_boxptr* ob)
{
    struct boxptrimpl_* impl = AUG_PODIMPL(struct boxptrimpl_, boxptr_, ob);
    ++impl->refs_;
}

static void
releaseboxptr_(aug_boxptr* ob)
{
    struct boxptrimpl_* impl = AUG_PODIMPL(struct boxptrimpl_, boxptr_, ob);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        if (impl->destroy_)
            impl->destroy_(impl->p_);
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static void*
getboxptr_(aug_boxptr* ob)
{
    struct boxptrimpl_* impl = AUG_PODIMPL(struct boxptrimpl_, boxptr_, ob);
    return impl->p_;
}

static const struct aug_boxptrvtbl boxptrvtbl_ = {
    castboxptr_,
    retainboxptr_,
    releaseboxptr_,
    getboxptr_
};


struct blobimpl_ {
    aug_blob blob_;
    int refs_;
    aug_mpool* mpool_;
    size_t size_;
    char buf_[1];
};

static void*
castblob_(aug_blob* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retainblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    ++impl->refs_;
}

static void
releaseblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static const void*
getblobdata_(aug_blob* ob, size_t* size)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (size)
        *size = impl->size_;
    return impl->buf_;
}

static size_t
getblobsize_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    return impl->size_;
}

static const struct aug_blobvtbl blobvtbl_ = {
    castblob_,
    retainblob_,
    releaseblob_,
    getblobdata_,
    getblobsize_
};

AUGUTIL_API aug_boxint*
aug_createboxint(aug_mpool* mpool, int i, void (*destroy)(int))
{
    struct boxintimpl_* impl = aug_allocmem(mpool,
                                            sizeof(struct boxintimpl_));
    if (!impl)
        return NULL;

    impl->boxint_.vtbl_ = &boxintvtbl_;
    impl->boxint_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->destroy_ = destroy;
    impl->i_ = i;

    aug_retain(mpool);
    return &impl->boxint_;
}

AUGUTIL_API int
aug_obtoi(aug_object* ob)
{
    int i;
    aug_boxint* tmp;
    if (ob && (tmp = aug_cast(ob, aug_boxintid))) {
        i = aug_getboxint(tmp);
        aug_release(tmp);
    } else
        i = 0;
    return i;
}

AUGUTIL_API aug_boxptr*
aug_createboxptr(aug_mpool* mpool, void* p, void (*destroy)(void*))
{
    struct boxptrimpl_* impl = aug_allocmem(mpool,
                                            sizeof(struct boxptrimpl_));
    if (!impl)
        return NULL;

    impl->boxptr_.vtbl_ = &boxptrvtbl_;
    impl->boxptr_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->destroy_ = destroy;
    impl->p_ = p;

    aug_retain(mpool);
    return &impl->boxptr_;
}

AUGUTIL_API void*
aug_obtop(aug_object* ob)
{
    void* p;
    aug_boxptr* tmp;
    if (ob && (tmp = aug_cast(ob, aug_boxptrid))) {
        p = aug_getboxptr(tmp);
        aug_release(tmp);
    } else
        p = NULL;
    return p;
}

AUGUTIL_API aug_blob*
aug_createblob(aug_mpool* mpool, const void* buf, size_t len)
{
    struct blobimpl_* impl = aug_allocmem(mpool,
                                          sizeof(struct blobimpl_) + len);
    if (!impl)
        return NULL;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->size_ = len;
    memcpy(impl->buf_, buf, len);
    impl->buf_[len] = '\0'; /* Null terminator. */

    aug_retain(mpool);
    return &impl->blob_;
}
