/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

#include <stdio.h>

typedef aug_result (*aug_confcb_t)(void*, const char*, const char*);

/**
 * Read configuration file.
 *
 * A negative callback result (an exception) will cause aug_readconf() to exit
 * with the same result.
 *
 * @param fp File handle to configuration file.
 *
 * @param cb Callback function called for each name/value pair.
 *
 * @param arg User specfied argument.
 */

AUGUTIL_API aug_result
aug_freadconf(FILE* fp, aug_confcb_t cb, void* arg);

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
