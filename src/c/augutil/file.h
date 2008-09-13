/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_FILE_H
#define AUGUTIL_FILE_H

/**
 * @file augutil/file.h
 *
 * Config parser.
 */

#include "augutil/config.h"

#include "augtypes.h"

typedef aug_result (*aug_confcb_t)(void*, const char*, const char*);

/**
 * Read configuration file.
 *
 * A negative callback result (an exception) will cause aug_readconf() to exit
 * with the same result.
 *
 * @param path Path to configuration file.
 *
 * @param cb Callback function called for each name/value pair.
 *
 * @param arg User specfied argument.
 */

AUGUTIL_API aug_result
aug_readconf(const char* path, aug_confcb_t cb, void* arg);

#endif /* AUGUTIL_FILE_H */
