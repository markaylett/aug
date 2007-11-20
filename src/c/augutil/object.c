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

struct intobjimpl_ {
    struct aug_intobj intobj_;
    unsigned refs_;
    void (*destroy_)(int);
    int i_;
};

static void*
castintobj_(aug_intobj_t obj, const char* type)
{
    if (0 == strcmp(type, "aug_object") || 0 == strcmp(type, "aug_intobj")) {
        obj->vtbl_->retain_(obj);
        return obj;
    }
    return NULL;
}

static int
retainintobj_(aug_intobj_t obj)
{
    struct intobjimpl_* impl = AUG_IMPL(struct intobjimpl_, intobj_, obj);
    ++impl->refs_;
    return 0;
}

static int
releaseintobj_(aug_intobj_t obj)
{
    struct intobjimpl_* impl = AUG_IMPL(struct intobjimpl_, intobj_, obj);
    if (0 == --impl->refs_ && impl->destroy_)
        impl->destroy_(impl->i_);
    return 0;
}

static int
getintobj_(aug_intobj_t obj)
{
    struct intobjimpl_* impl = AUG_IMPL(struct intobjimpl_, intobj_, obj);
    return impl->i_;
}

static const struct aug_intobjvtbl intobjvtbl_ = {
    castintobj_,
    retainintobj_,
    releaseintobj_,
    getintobj_
};

AUGUTIL_API aug_intobj_t
aug_createintobj(int i, void (*destroy)(int))
{
    struct intobjimpl_* impl = malloc(sizeof(struct intobjimpl_));
    if (!impl) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    impl->intobj_.vtbl_ = &intobjvtbl_;
    impl->refs_ = 1;
    impl->destroy_ = destroy;
    impl->i_ = i;

    return &impl->intobj_;
}

AUGUTIL_API int
aug_objtoint(aug_object_t obj)
{
    int i;
    aug_intobj_t tmp;
    if (obj && (tmp = aug_castobject(obj, "aug_intobj"))) {
        i = aug_getintobj(tmp);
        aug_releaseobject(tmp);
    } else
        i = 0;
    return i;
}

struct ptrobjimpl_ {
    struct aug_ptrobj ptrobj_;
    unsigned refs_;
    void (*destroy_)(void*);
    void* p_;
};

static void*
castptrobj_(aug_ptrobj_t obj, const char* type)
{
    if (0 == strcmp(type, "aug_object") || 0 == strcmp(type, "aug_ptrobj")) {
        obj->vtbl_->retain_(obj);
        return obj;
    }
    return NULL;
}

static int
retainptrobj_(aug_ptrobj_t obj)
{
    struct ptrobjimpl_* impl = AUG_IMPL(struct ptrobjimpl_, ptrobj_, obj);
    ++impl->refs_;
    return 0;
}

static int
releaseptrobj_(aug_ptrobj_t obj)
{
    struct ptrobjimpl_* impl = AUG_IMPL(struct ptrobjimpl_, ptrobj_, obj);
    if (0 == --impl->refs_ && impl->destroy_)
        impl->destroy_(impl->p_);
    return 0;
}

static void*
getptrobj_(aug_ptrobj_t obj)
{
    struct ptrobjimpl_* impl = AUG_IMPL(struct ptrobjimpl_, ptrobj_, obj);
    return impl->p_;
}

static const struct aug_ptrobjvtbl ptrobjvtbl_ = {
    castptrobj_,
    retainptrobj_,
    releaseptrobj_,
    getptrobj_
};

AUGUTIL_API aug_ptrobj_t
aug_createptrobj(void* p, void (*destroy)(void*))
{
    struct ptrobjimpl_* impl = malloc(sizeof(struct ptrobjimpl_));
    if (!impl) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    impl->ptrobj_.vtbl_ = &ptrobjvtbl_;
    impl->refs_ = 1;
    impl->destroy_ = destroy;
    impl->p_ = p;

    return &impl->ptrobj_;
}

AUGUTIL_API void*
aug_objtoptr(aug_object_t obj)
{
    void* p;
    aug_ptrobj_t tmp;
    if (obj && (tmp = aug_castobject(obj, "aug_objptr"))) {
        p = aug_getptrobj(tmp);
        aug_releaseobject(tmp);
    } else
        p = NULL;
    return p;
}
