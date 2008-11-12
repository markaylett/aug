/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_NETWORDS_H
#define AUGUTIL_NETWORDS_H

#include "augutil/config.h"
#include "augutil/types.h"
#include "augsys/types.h" /* size_t */

AUGUTIL_API void
aug_initnetwords(struct aug_words* st, void (*out)(void*, int), void* arg);

AUGUTIL_API void
aug_putnetwords(struct aug_words* st, int ch);

AUGUTIL_API size_t
aug_rtrimword(const char* src, size_t size);

#endif /* AUGUTIL_NETWORDS_H */
