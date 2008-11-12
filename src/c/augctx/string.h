/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
