/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_OBJECT_H
#define AUGUTIL_OBJECT_H

/**
 * @file augutil/object.h
 *
 * Object implementations.
 */

#include "augutil/config.h"

#include "augob/addrob.h"
#include "augob/blob.h"
#include "augob/longob.h"

AUGUTIL_API aug_longob*
aug_createlongob(long l, void (*destroy)(long));

AUGUTIL_API long
aug_obtolong(aug_object* ob);

/**
 * Create object wrapper for plain pointer.
 *
 * @param p Plain pointer.
 *
 * @param destroy Destructor to be called when reference count reaches zero.
 *
 * @return Address of new object, or null on error.
 */

AUGUTIL_API aug_addrob*
aug_createaddrob(void* p, void (*destroy)(void*));

/**
 * Cast to @ref aug_addrob and return plain pointer.
 *
 * @param ob Base object address.
 *
 * @return Plain pointer, or null if not @ref aug_addrob type.
 */

AUGUTIL_API void*
aug_obtoaddr(aug_object* ob);

AUGUTIL_API aug_blob*
aug_createblob(const void* buf, size_t len);

#endif /* AUGUTIL_OBJECT_H */
