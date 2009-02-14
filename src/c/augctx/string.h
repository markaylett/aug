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
#ifndef AUGCTX_STRING_H
#define AUGCTX_STRING_H

/**
 * @file augctx/string.h
 *
 * Portable c-string functions.
 */

#include "augctx/config.h"

#include <string.h>

/**
 * Copies string from @a src to @a dst, ensuring that resulting string is
 * null-terminated.
 *
 * @param dst Destination buffer.
 *
 * @param src Source buffer.
 *
 * @param size Size of @a dst buffer.
 *
 * @return The length of @a src.
 */

AUGCTX_API size_t
aug_strlcpy(char* dst, const char* src, size_t size);

/**
 * Case insensitive comparison.
 *
 * @param lhs First string.
 *
 * @param rhs Second string.
 *
 * @return Less than zero if @a lhs is less than @a rhs, greater than zero if
 * @a lhs is greater than @a rhs, or zero if both are equal.
 */

AUGCTX_API int
aug_strcasecmp(const char* lhs, const char* rhs);

/**
 * Case insensitive comparison of not more than @a size characters.
 *
 * @param lhs First string.
 *
 * @param rhs Second string.
 *
 * @param size Maximum number of characters to be compared.
 *
 * @return Less than zero if @a lhs is less than @a rhs, greater than zero if
 * @a lhs is greater than @a rhs, or zero if both are equal.
 */

AUGCTX_API int
aug_strncasecmp(const char* lhs, const char* rhs, size_t size);

/**
 * Case insensitive search for @a needle substring in @a haystack string.
 *
 * @param haystack String to be searched.
 *
 * @param needle Substring to be searched for.
 *
 * @return Pointer to first character of @a needle substring in @a haystack.
 */

AUGCTX_API const char*
aug_strcasestr(const char* haystack, const char* needle);

#endif /* AUGCTX_STRING_H */
