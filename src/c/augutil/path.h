/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_PATH_H
#define AUGUTIL_PATH_H

/**
 * @file augutil/object.h
 *
 * Pathname functions.
 */

#include "augutil/config.h"

#include "augsys/types.h"

#include "augtypes.h"

AUGUTIL_API const char*
aug_basename(const char* path);

AUGUTIL_API aug_result
aug_chdir(const char* path);

AUGUTIL_API char*
aug_getcwd(char* dst, size_t size);

AUGUTIL_API char*
aug_gethome(char* dst, size_t size);

AUGUTIL_API char*
aug_gettmp(char* dst, size_t size);

AUGUTIL_API char*
aug_getrundir(char* dst, size_t size);

AUGUTIL_API char*
aug_makepath(char* dst, const char* dir, const char* name, const char* ext,
             size_t size);

AUGUTIL_API char*
aug_realpath(char* dst, const char* src, size_t size);

#endif /* AUGUTIL_PATH_H */
