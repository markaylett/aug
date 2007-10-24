/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_NETWORDS_H
#define AUGUTIL_NETWORDS_H

#include "augutil/config.h"
#include "augutil/types.h"
#include "augsys/types.h" /* size_t */

AUGUTIL_API void
aug_initnetwords(struct words* st, void (*out)(void*, int), void* arg);

AUGUTIL_API void
aug_putnetwords(struct words* st, int ch);

AUGUTIL_API size_t
aug_rtrimword(const char* src, size_t size);

#endif /* AUGUTIL_NETWORDS_H */
