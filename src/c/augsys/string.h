/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_STRING_H
#define AUGSYS_STRING_H

/**
 * @file augsys/string.h
 *
 * Portable c-string functions.
 *
 * Functions in this module may set errno, but never errinfo.
 */

#include "augsys/config.h"

#include <string.h>

/**
 * Maps @a errnum to a descriptive error message.
 *
 * @param errnum The error number.
 *
 * @return The description.
 */

AUGSYS_API const char*
aug_strerror(int errnum);

/**
 * Scramble or unscramble @a dst buffer.
 *
 * Useful for hiding memory contents, but not true encryption.
 *
 * @param dst The buffer.
 *
 * @param size Size of @a dst buffer.
 *
 * @return @a dst buffer.
 */

AUGSYS_API void*
aug_memfrob(void* dst, size_t size);

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

AUGSYS_API size_t
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

AUGSYS_API int
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

AUGSYS_API int
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

AUGSYS_API const char*
aug_strcasestr(const char* haystack, const char* needle);

#endif /* AUGSYS_STRING_H */
