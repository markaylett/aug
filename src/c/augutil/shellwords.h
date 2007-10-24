/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_SHELLWORDS_H
#define AUGUTIL_SHELLWORDS_H

#include "augutil/config.h"
#include "augutil/types.h"

AUGUTIL_API void
aug_initshellwords(struct words* st, void (*out)(void*, int), void* arg);

AUGUTIL_API void
aug_putshellwords(struct words* st, int ch);

#endif /* AUGUTIL_SHELLWORDS_H */
