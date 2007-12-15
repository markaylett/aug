/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/object.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"
#include "augsys/errno.h"

#include <stdlib.h> /* malloc() */
#include <string.h>

struct longobimpl_ {
    aug_longob longob_;
    unsigned refs_;
    void (*destroy_)(long);
    long l_;
};

static void*
castlongob_(aug_longob* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_longobid)) {
        aug_incref(ob);
        return ob;
    }
    return NULL;
}

static int
increflongob_(aug_longob* ob)
{
    struct longobimpl_* impl = AUG_PODIMPL(struct longobimpl_, longob_, ob);
    ++impl->refs_;
    return 0;
}

static int
decreflongob_(aug_longob* ob)
{
    struct longobimpl_* impl = AUG_PODIMPL(struct longobimpl_, longob_, ob);
    if (0 == --impl->refs_) {
        if (impl->destroy_)
            impl->destroy_(impl->l_);
        free(impl);
    }
    return 0;
}

static long
getlongob_(aug_longob* ob)
{
    struct longobimpl_* impl = AUG_PODIMPL(struct longobimpl_, longob_, ob);
    return impl->l_;
}

static const struct aug_longobvtbl longobvtbl_ = {
    castlongob_,
    increflongob_,
    decreflongob_,
    getlongob_
};

struct addrobimpl_ {
    aug_addrob addrob_;
    unsigned refs_;
    void (*destroy_)(void*);
    void* p_;
};

static void*
castaddrob_(aug_addrob* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_addrobid)) {
        aug_incref(ob);
        return ob;
    }
    return NULL;
}

static int
increfaddrob_(aug_addrob* ob)
{
    struct addrobimpl_* impl = AUG_PODIMPL(struct addrobimpl_, addrob_, ob);
    ++impl->refs_;
    return 0;
}

static int
decrefaddrob_(aug_addrob* ob)
{
    struct addrobimpl_* impl = AUG_PODIMPL(struct addrobimpl_, addrob_, ob);
    if (0 == --impl->refs_) {
        if (impl->destroy_)
            impl->destroy_(impl->p_);
        free(impl);
    }
    return 0;
}

static void*
getaddrob_(aug_addrob* ob)
{
    struct addrobimpl_* impl = AUG_PODIMPL(struct addrobimpl_, addrob_, ob);
    return impl->p_;
}

static const struct aug_addrobvtbl addrobvtbl_ = {
    castaddrob_,
    increfaddrob_,
    decrefaddrob_,
    getaddrob_
};


struct blobimpl_ {
    aug_blob blob_;
    unsigned refs_;
    size_t size_;
    char buf_[1];
};

static void*
castblob_(aug_blob* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_incref(ob);
        return ob;
    }
    return NULL;
}

static int
increfblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    ++impl->refs_;
    return 0;
}

static int
decrefblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (0 == --impl->refs_)
        free(impl);
    return 0;
}

static const void*
blobdata_(aug_blob* ob, size_t* size)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (size)
        *size = impl->size_;
    return impl->buf_;
}

static size_t
blobsize_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    return impl->size_;
}

static const struct aug_blobvtbl blobvtbl_ = {
    castblob_,
    increfblob_,
    decrefblob_,
    blobdata_,
    blobsize_
};

AUGUTIL_API aug_longob*
aug_createlongob(long l, void (*destroy)(long))
{
    struct longobimpl_* impl = malloc(sizeof(struct longobimpl_));
    if (!impl) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    impl->longob_.vtbl_ = &longobvtbl_;
    impl->longob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->destroy_ = destroy;
    impl->l_ = l;

    return &impl->longob_;
}

AUGUTIL_API long
aug_obtolong(aug_object* ob)
{
    long l;
    aug_longob* tmp;
    if (ob && (tmp = aug_cast(ob, aug_longobid))) {
        l = aug_getlongob(tmp);
        aug_decref(tmp);
    } else
        l = 0;
    return l;
}

AUGUTIL_API aug_addrob*
aug_createaddrob(void* p, void (*destroy)(void*))
{
    struct addrobimpl_* impl = malloc(sizeof(struct addrobimpl_));
    if (!impl) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    impl->addrob_.vtbl_ = &addrobvtbl_;
    impl->addrob_.impl_ = NULL;
    impl->refs_ = 1;
    impl->destroy_ = destroy;
    impl->p_ = p;

    return &impl->addrob_;
}

AUGUTIL_API void*
aug_obtoaddr(aug_object* ob)
{
    void* p;
    aug_addrob* tmp;
    if (ob && (tmp = aug_cast(ob, aug_addrobid))) {
        p = aug_getaddrob(tmp);
        aug_decref(tmp);
    } else
        p = NULL;
    return p;
}

AUGUTIL_API aug_blob*
aug_createblob(const void* buf, size_t len)
{
    struct blobimpl_* impl = malloc(sizeof(struct blobimpl_) + len - 1);
    if (!impl) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->refs_ = 1;
    impl->size_ = len;
    memcpy(impl->buf_, buf, len);

    return &impl->blob_;
}
