/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_FILE_H
#define AUGUTIL_FILE_H

#include "augutil/config.h"

typedef int (*aug_setopt_t)(void*, const char*, const char*);

AUGUTIL_API int
aug_readconf(const char* path, aug_setopt_t setopt, void* arg);

#endif /* AUGUTIL_FILE_H */
