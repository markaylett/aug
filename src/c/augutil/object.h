/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_OBJECT_H
#define AUGUTIL_OBJECT_H

#include "augutil/config.h"

#include "augobj/addrob.h"
#include "augobj/blob.h"
#include "augobj/longob.h"

AUGUTIL_API aug_longob*
aug_createlongob(long l, void (*destroy)(long));

AUGUTIL_API long
aug_obtolong(aug_object* obj);

AUGUTIL_API aug_addrob*
aug_createaddrob(void* p, void (*destroy)(void*));

AUGUTIL_API void*
aug_obtoaddr(aug_object* obj);

AUGUTIL_API aug_blob*
aug_createblob(const void* buf, size_t len);

#endif /* AUGUTIL_OBJECT_H */
