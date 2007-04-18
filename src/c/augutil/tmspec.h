/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_TMSPEC_H
#define AUGUTIL_TMSPEC_H

#include "augutil/config.h"

struct tm;

struct aug_tmspec {
    int min_, hour_, mday_, mon_, wday_;
};

AUGUTIL_API struct aug_tmspec*
aug_strtmspec(struct aug_tmspec* tms, const char* s);

AUGUTIL_API struct tm*
aug_nexttime(struct tm* tm, const struct aug_tmspec* tms);

#endif /* AUGUTIL_TMSPEC_H */
