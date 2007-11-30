/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_OBJECT_H
#define AUGUTIL_OBJECT_H

#include "augutil/config.h"

#include "augobj.h"

AUG_OBJECTDECL(aug_longob);
struct aug_longobvtbl {
    AUG_OBJECT(aug_longob);
    long (*get_)(aug_longob*);
};

#define aug_getlongob(obj) \
    ((aug_longob*)obj)->vtbl_->get_(obj)

AUG_OBJECTDECL(aug_addrob);
struct aug_addrobvtbl {
    AUG_OBJECT(aug_addrob);
    void* (*get_)(aug_addrob*);
};

#define aug_getaddrob(obj) \
    ((aug_addrob*)obj)->vtbl_->get_(obj)

AUGUTIL_API aug_longob*
aug_createlongob(long l, void (*destroy)(long));

AUGUTIL_API long
aug_obtolong(aug_object* obj);

AUGUTIL_API aug_addrob*
aug_createaddrob(void* p, void (*destroy)(void*));

AUGUTIL_API void*
aug_obtoaddr(aug_object* obj);

#endif /* AUGUTIL_OBJECT_H */
