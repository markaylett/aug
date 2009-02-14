/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGUTIL_XSTR_H
#define AUGUTIL_XSTR_H

/**
 * @file augutil/xstr.h
 *
 * Dynamic string type.
 */

#include "augutil/config.h"

#include "augsys/types.h" /* size_t */

#include "augext/mpool.h"
#include "augext/stream.h"

#include "augtypes.h"

typedef struct aug_xstr_* aug_xstr_t;

AUGUTIL_API aug_xstr_t
aug_createxstr(aug_mpool* mpool, size_t size);

AUGUTIL_API void
aug_destroyxstr(aug_xstr_t xstr);

AUGUTIL_API aug_result
aug_clearxstrn(aug_xstr_t xstr, size_t len);

AUGUTIL_API aug_result
aug_clearxstr(aug_xstr_t xstr);

AUGUTIL_API aug_result
aug_xstrcatsn(aug_xstr_t xstr, const char* src, size_t len);

AUGUTIL_API aug_result
aug_xstrcats(aug_xstr_t xstr, const char* src);

/**
 * Concatenate strings.  Safe to use when both arguments are the same string.
 *
 * @param xstr Destination string.
 * @param src Source string.
 *
 * @return Either @ref AUG_SUCCESS or @ref AUG_FAILERROR.
 */

AUGUTIL_API aug_result
aug_xstrcat(aug_xstr_t xstr, const aug_xstr_t src);

AUGUTIL_API aug_result
aug_xstrcpysn(aug_xstr_t xstr, const char* src, size_t len);

AUGUTIL_API aug_result
aug_xstrcpys(aug_xstr_t xstr, const char* src);

/**
 * Copy strings.  Safe to use when both arguments are the same string.
 *
 * @param xstr Destination string.
 * @param src Source string.
 *
 * @return Either @ref AUG_SUCCESS or @ref AUG_FAILERROR.
 */

AUGUTIL_API aug_result
aug_xstrcpy(aug_xstr_t xstr, const aug_xstr_t src);

AUGUTIL_API aug_result
aug_xstrcatcn(aug_xstr_t xstr, char ch, size_t num);

AUGUTIL_API aug_result
aug_xstrcatc(aug_xstr_t xstr, char ch);

AUGUTIL_API aug_result
aug_xstrcpycn(aug_xstr_t xstr, char ch, size_t num);

AUGUTIL_API aug_result
aug_xstrcpyc(aug_xstr_t xstr, char ch);

AUGUTIL_API aug_rsize
aug_xstrread(aug_xstr_t xstr, aug_stream* src, size_t size);

AUGUTIL_API size_t
aug_xstrlen(aug_xstr_t xstr);

AUGUTIL_API const char*
aug_xstr(aug_xstr_t xstr);

#endif /* AUGUTIL_XSTR_H */
