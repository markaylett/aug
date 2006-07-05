/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_FILE_H
#define AUGUTIL_FILE_H

#include "augutil/var.h"

typedef int (*aug_setopt_t)(const struct aug_var*, const char*, const char*);

AUGUTIL_API int
aug_readconf(const char* path, aug_setopt_t setopt,
             const struct aug_var* arg);

#endif /* AUGUTIL_FILE_H */
