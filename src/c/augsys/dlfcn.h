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
#ifndef AUGSYS_DLFCN_H
#define AUGSYS_DLFCN_H

/**
 * @file augsys/dlfcn.h
 *
 * Dynamic linker support.
 */

#include "augsys/config.h"

#include "augext/mpool.h"

#include "augtypes.h"

typedef void (*aug_fnptr_t)();
typedef struct aug_dlib_* aug_dlib_t;

AUGSYS_API aug_result
aug_dlclose(aug_dlib_t dlib);

AUGSYS_API aug_dlib_t
aug_dlopen(aug_mpool* mpool, const char* path);

AUGSYS_API aug_fnptr_t
aug_dlsym(aug_dlib_t dlib, const char* symbol);

#endif /* AUGSYS_DLFCN_H */
