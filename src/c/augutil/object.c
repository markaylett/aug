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
    aug_intobj intobj_;
    unsigned refs_;
    void (*destroy_)(int);
    int i_;
};

static void*
castintobj_(aug_intobj* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_intobjid)) {
        aug_incref(obj);
        return obj;
    }
    return NULL;
}

static int
increfintobj_(aug_intobj* obj)
{
    struct intobjimpl_* impl = AUG_PODIMPL(struct intobjimpl_, intobj_, obj);
    ++impl->refs_;
    return 0;
}

static int
decrefintobj_(aug_intobj* obj)
{
    struct intobjimpl_* impl = AUG_PODIMPL(struct intobjimpl_, intobj_, obj);
    if (0 == --impl->refs_) {
        if (impl->destroy_)
            impl->destroy_(impl->i_);
        free(impl);
    }
    return 0;
}

static int
getintobj_(aug_intobj* obj)
{
    struct intobjimpl_* impl = AUG_PODIMPL(struct intobjimpl_, intobj_, obj);
    return impl->i_;
}

static const struct aug_intobjvtbl intobjvtbl_ = {
    castintobj_,
    increfintobj_,
    decrefintobj_,
    getintobj_
};

AUGUTIL_API aug_intobj*
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
aug_objtoint(aug_object* obj)
{
    int i;
    aug_intobj* tmp;
    if (obj && (tmp = aug_cast(obj, aug_intobjid))) {
        i = aug_getintobj(tmp);
        aug_decref(tmp);
    } else
        i = 0;
    return i;
}

struct ptrobjimpl_ {
    aug_ptrobj ptrobj_;
    unsigned refs_;
    void (*destroy_)(void*);
    void* p_;
};

static void*
castptrobj_(aug_ptrobj* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_ptrobjid)) {
        aug_incref(obj);
        return obj;
    }
    return NULL;
}

static int
increfptrobj_(aug_ptrobj* obj)
{
    struct ptrobjimpl_* impl = AUG_PODIMPL(struct ptrobjimpl_, ptrobj_, obj);
    ++impl->refs_;
    return 0;
}

static int
decrefptrobj_(aug_ptrobj* obj)
{
    struct ptrobjimpl_* impl = AUG_PODIMPL(struct ptrobjimpl_, ptrobj_, obj);
    if (0 == --impl->refs_) {
        if (impl->destroy_)
            impl->destroy_(impl->p_);
        free(impl);
    }
    return 0;
}

static void*
getptrobj_(aug_ptrobj* obj)
{
    struct ptrobjimpl_* impl = AUG_PODIMPL(struct ptrobjimpl_, ptrobj_, obj);
    return impl->p_;
}

static const struct aug_ptrobjvtbl ptrobjvtbl_ = {
    castptrobj_,
    increfptrobj_,
    decrefptrobj_,
    getptrobj_
};

AUGUTIL_API aug_ptrobj*
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
aug_objtoptr(aug_object* obj)
{
    void* p;
    aug_ptrobj* tmp;
    if (obj && (tmp = aug_cast(obj, aug_ptrobjid))) {
        p = aug_getptrobj(tmp);
        aug_decref(tmp);
    } else
        p = NULL;
    return p;
}
