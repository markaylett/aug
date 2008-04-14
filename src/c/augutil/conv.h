/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_CONV_H
#define AUGUTIL_CONV_H

/**
 * @file augutil/conv.h
 *
 * String conversion functions.
 */

#include "augutil/config.h"

AUGUTIL_API unsigned long*
aug_strtoul(unsigned long* dst, const char* src, int base);

AUGUTIL_API unsigned*
aug_strtoui(unsigned* dst, const char* src, int base);

AUGUTIL_API unsigned short*
aug_strtous(unsigned short* dst, const char* src, int base);

#endif /* AUGUTIL_CONV_H */
