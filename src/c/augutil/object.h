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

#include "augext/blob.h"
#include "augext/boxint.h"
#include "augext/boxptr.h"

/**
 * Create object wrapper for integer.
 *
 * @param i Integer.
 *
 * @param destroy Destructor to be called when reference count reaches zero.
 *
 * @return Address of new object, or null on error.
 */

AUGUTIL_API aug_boxint*
aug_createboxint(int i, void (*destroy)(int));

/**
 * Cast to @ref aug_boxint and return boxed integer.
 *
 * @param ob Base object address.
 *
 * @return Boxed integer, or zero if not @ref aug_boxint type.
 */

AUGUTIL_API int
aug_obtoi(aug_object* ob);

/**
 * Create object wrapper for plain pointer.
 *
 * @param p Plain pointer.
 *
 * @param destroy Destructor to be called when reference count reaches zero.
 *
 * @return Address of new object, or null on error.
 */

AUGUTIL_API aug_boxptr*
aug_createboxptr(void* p, void (*destroy)(void*));

/**
 * Cast to @ref aug_boxptr and return plain pointer.
 *
 * @param ob Base object address.
 *
 * @return Plain pointer, or null if not @ref aug_boxptr type.
 */

AUGUTIL_API void*
aug_obtop(aug_object* ob);

AUGUTIL_API aug_blob*
aug_createblob(const void* buf, size_t len);

#endif /* AUGUTIL_OBJECT_H */
