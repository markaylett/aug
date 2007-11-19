/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_OBJECT_H
#define AUGUTIL_OBJECT_H

#include "augutil/config.h"

#include "augobj.h"

/* aug_intobj */

struct aug_intobjvtbl;

typedef struct aug_intobj {
    const struct aug_intobjvtbl* vtbl_;
}* aug_intobj_t;

#define aug_intobjtype() "aug_intobj"

struct aug_intobjvtbl {
    AUG_OBJECT(aug_intobj);
    int (*get_)(aug_intobj_t);
};

#define aug_getintobj(obj)                      \
    (obj)->vtbl_->get_(obj)

AUGUTIL_API aug_intobj_t
aug_createintobj(int i, void (*destroy)(int));

AUGUTIL_API int
aug_objtoint(aug_object_t obj);

/* aug_ptrobj */

struct aug_ptrobjvtbl;

typedef struct aug_ptrobj {
    const struct aug_ptrobjvtbl* vtbl_;
}* aug_ptrobj_t;

#define aug_ptrobjtype() "aug_ptrobj"

struct aug_ptrobjvtbl {
    AUG_OBJECT(aug_ptrobj);
    void* (*get_)(aug_ptrobj_t);
};

#define aug_getptrobj(obj)                      \
    (obj)->vtbl_->get_(obj)

AUGUTIL_API aug_ptrobj_t
aug_createptrobj(void* p, void (*destroy)(void*));

AUGUTIL_API void*
aug_objtoptr(aug_object_t obj);

#endif /* AUGUTIL_OBJECT_H */
