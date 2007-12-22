/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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

typedef struct aug_xstr_* aug_xstr_t;

AUGUTIL_API aug_xstr_t
aug_createxstr(size_t size);

AUGUTIL_API int
aug_destroyxstr(aug_xstr_t xstr);

AUGUTIL_API int
aug_clearxstrn(aug_xstr_t* xstr, size_t len);

AUGUTIL_API int
aug_clearxstr(aug_xstr_t* xstr);

AUGUTIL_API int
aug_xstrcatsn(aug_xstr_t* xstr, const char* src, size_t len);

AUGUTIL_API int
aug_xstrcats(aug_xstr_t* xstr, const char* src);

AUGUTIL_API int
aug_xstrcat(aug_xstr_t* xstr, const aug_xstr_t src);

AUGUTIL_API int
aug_xstrcpysn(aug_xstr_t* xstr, const char* src, size_t len);

AUGUTIL_API int
aug_xstrcpys(aug_xstr_t* xstr, const char* src);

AUGUTIL_API int
aug_xstrcpy(aug_xstr_t* xstr, const aug_xstr_t src);

AUGUTIL_API int
aug_xstrcatcn(aug_xstr_t* xstr, char ch, size_t num);

AUGUTIL_API int
aug_xstrcatc(aug_xstr_t* xstr, char ch);

AUGUTIL_API ssize_t
aug_xstrcatf(int fd, aug_xstr_t* xstr, size_t size);

AUGUTIL_API int
aug_xstrcpycn(aug_xstr_t* xstr, char ch, size_t num);

AUGUTIL_API int
aug_xstrcpyc(aug_xstr_t* xstr, char ch);

AUGUTIL_API size_t
aug_xstrlen(aug_xstr_t xstr);

AUGUTIL_API const char*
aug_xstr(aug_xstr_t xstr);

#endif /* AUGUTIL_XSTR_H */
