/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_FILE_H
#define AUGUTIL_FILE_H

#include "augutil/config.h"

typedef int (*aug_confcb_t)(void*, const char*, const char*);

/**
   Read configuration file.

   \param path Path to configuration file.
   \param cb Callback function called for each name/value pair.
   \param arg User specfied argument.
 */

AUGUTIL_API int
aug_readconf(const char* path, aug_confcb_t cb, void* arg);

#endif /* AUGUTIL_FILE_H */
