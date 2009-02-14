/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
