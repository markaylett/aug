/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_OBJECT_H
#define AUGUTIL_OBJECT_H

#include "augutil/config.h"

#include "augobj.h"

/* aug_intobj */

AUG_OBJECTDECL(aug_intobj);

struct aug_intobjvtbl {
    AUG_OBJECT(aug_intobj);
    int (*get_)(aug_intobj*);
};

#define aug_getintobj(obj)                      \
    ((aug_intobj*)obj)->vtbl_->get_(obj)

AUGUTIL_API aug_intobj*
aug_createintobj(int i, void (*destroy)(int));

AUGUTIL_API int
aug_objtoint(aug_object* obj);

/* aug_ptrobj */

AUG_OBJECTDECL(aug_ptrobj);

struct aug_ptrobjvtbl {
    AUG_OBJECT(aug_ptrobj);
    void* (*get_)(aug_ptrobj*);
};

#define aug_getptrobj(obj)                      \
    ((aug_ptrobj*)obj)->vtbl_->get_(obj)

AUGUTIL_API aug_ptrobj*
aug_createptrobj(void* p, void (*destroy)(void*));

AUGUTIL_API void*
aug_objtoptr(aug_object* obj);

#endif /* AUGUTIL_OBJECT_H */
