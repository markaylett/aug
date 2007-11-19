/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_OBJECT_H
#define AUGUTIL_OBJECT_H

#include "augutil/config.h"

#include "augobj.h"

typedef struct aug_intobj* aug_intobj_t;

struct aug_intobjvtbl {
    AUG_OBJECT(aug_intobj);
    int (*get_)(aug_intobj_t);
};

struct aug_intobj {
    const struct aug_intobjvtbl* vtbl_;
};

#define aug_castintobj(obj, type)               \
    (obj)->vtbl_->cast_(onbj, type)

#define aug_retainintobj(obj)                   \
    (obj)->vtbl_->retain_(obj)

#define aug_releaseintobj(obj)                  \
    (obj)->vtbl_->release_(obj)

#define aug_getintobj(obj)                      \
    (obj)->vtbl_->get_(obj)

AUGUTIL_API aug_intobj_t
aug_createintobj(int i, void (*destroy)(int));

typedef struct aug_ptrobj* aug_ptrobj_t;

struct aug_ptrobjvtbl {
    AUG_OBJECT(aug_ptrobj);
    void* (*get_)(aug_ptrobj_t);
};

struct aug_ptrobj {
    const struct aug_ptrobjvtbl* vtbl_;
};

#define aug_castptrobj(obj, type)               \
    (obj)->vtbl_->cast_(onbj, type)

#define aug_retainptrobj(obj)                   \
    (obj)->vtbl_->retain_(obj)

#define aug_releaseptrobj(obj)                  \
    (obj)->vtbl_->release_(obj)

#define aug_getptrobj(obj)                      \
    (obj)->vtbl_->get_(obj)

AUGUTIL_API aug_ptrobj_t
aug_createptrobj(void* p, void (*destroy)(void*));

#endif /* AUGUTIL_OBJECT_H */
