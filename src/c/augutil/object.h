/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGUTIL_OBJECT_H
#define AUGUTIL_OBJECT_H

/**
 * @file augutil/object.h
 *
 * Object implementations.
 */

#include "augutil/config.h"

#include "augext/array.h"
#include "augext/blob.h"
#include "augext/boxint.h"
#include "augext/boxptr.h"
#include "augext/mpool.h"

#include <stdarg.h>

/**
 * Create object wrapper for integer.
 *
 * @param mpool Memory pool.
 *
 * @param i Integer.
 *
 * @param destroy Destructor to be called when reference count reaches zero.
 *
 * @return Address of new object, or null on error.
 */

AUGUTIL_API aug_boxint*
aug_createboxint(aug_mpool* mpool, int i, void (*destroy)(int));

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
 * @param mpool Memory pool.
 *
 * @param p Plain pointer.
 *
 * @param destroy Destructor to be called when reference count reaches zero.
 *
 * @return Address of new object, or null on error.
 */

AUGUTIL_API aug_boxptr*
aug_createboxptr(aug_mpool* mpool, void* p, void (*destroy)(void*));

/**
 * Cast to @ref aug_boxptr and return plain pointer.
 *
 * @param ob Base object address.
 *
 * @return Plain pointer, or null if not @ref aug_boxptr type.
 */

AUGUTIL_API void*
aug_obtop(aug_object* ob);

/**
 * Create a blob with a copy of @a buf.
 */

AUGUTIL_API aug_blob*
aug_createblob(aug_mpool* mpool, const void* buf, size_t len);

/**
 * v - struct aug_var**;
 * i - int32_t*;
 * l - int64_t*;
 * d - double*;
 * b - aug_bool*;
 * O - aug_object**;
 * A - aug_array**;
 * B - aug_blob**;
 * S - aug_blob**;
 */

AUGUTIL_API aug_result
aug_vunpackargs(aug_array* array, const char* sig, va_list args);

/**
 * v - struct aug_var**;
 * i - int32_t*;
 * l - int64_t*;
 * d - double*;
 * b - aug_bool*;
 * O - aug_object**;
 * A - aug_array**;
 * B - aug_blob**;
 * S - aug_blob**;
 */

AUGUTIL_API aug_result
aug_unpackargs(aug_array* array, const char* sig, ...);

#endif /* AUGUTIL_OBJECT_H */
