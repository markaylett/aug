/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_DSTR_H
#define AUGUTIL_DSTR_H

#include "augutil/config.h"

#include "augsys/types.h" /* size_t */

typedef struct aug_dstr_* aug_dstr_t;

AUGUTIL_API aug_dstr_t
aug_createdstr(size_t size);

AUGUTIL_API int
aug_freedstr(aug_dstr_t dstr);

AUGUTIL_API int
aug_cleardstr(aug_dstr_t* dstr);

AUGUTIL_API int
aug_dstrcatsn(aug_dstr_t* dstr, const char* src, size_t len);

AUGUTIL_API int
aug_dstrcats(aug_dstr_t* dstr, const char* src);

AUGUTIL_API int
aug_dstrcat(aug_dstr_t* dstr, const aug_dstr_t src);

AUGUTIL_API int
aug_dstrsetsn(aug_dstr_t* dstr, const char* src, size_t len);

AUGUTIL_API int
aug_dstrsets(aug_dstr_t* dstr, const char* src);

AUGUTIL_API int
aug_dstrset(aug_dstr_t* dstr, const aug_dstr_t src);

AUGUTIL_API int
aug_dstrcatcn(aug_dstr_t* dstr, char ch, size_t num);

AUGUTIL_API int
aug_dstrcatc(aug_dstr_t* dstr, char ch);

AUGUTIL_API int
aug_dstrsetcn(aug_dstr_t* dstr, char ch, size_t num);

AUGUTIL_API int
aug_dstrsetc(aug_dstr_t* dstr, char ch);

AUGUTIL_API size_t
aug_dstrlen(aug_dstr_t dstr);

AUGUTIL_API const char*
aug_dstr(aug_dstr_t dstr);

#endif /* AUGUTIL_DSTR_H */
