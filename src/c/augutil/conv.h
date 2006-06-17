/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_CONV_H
#define AUGUTIL_CONV_H

#include "augutil/config.h"

AUGUTIL_API int
aug_strtoul(unsigned long* dst, const char* src, int base);

AUGUTIL_API int
aug_strtoui(unsigned int* dst, const char* src, int base);

AUGUTIL_API int
aug_strtous(unsigned short* dst, const char* src, int base);

#endif /* AUGUTIL_CONV_H */
