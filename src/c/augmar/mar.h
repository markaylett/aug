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
#ifndef AUGMAR_MAR_H
#define AUGMAR_MAR_H

/**
 * @file augmar/mar.h
 *
 * Meta ARchive file format.
 */

#include "augmar/config.h"
#include "augmar/types.h"

#include "augext/mar.h"
#include "augext/mpool.h"

/**
 * Create an in-memory message archive.
 *
 * @param mpool Memory pool.
 *
 * @return A handle to the newly created message archive or null on failure.
 *
 * @see aug_openmar() and aug_releasemar().
 */

AUGMAR_API aug_mar*
aug_createmar_AIN(aug_mpool* mpool);

/**
 * Create or open a file-based message archive.
 *
 * @param mpool Memory pool.
 *
 * @param path A path to the file to be created or opened.
 *
 * @param flags The @ref OpenFlags used to create or open the file.
 *
 * @param ... The permissions used to create a new file.  Required when the
 * #AUG_CREAT flag has been specified, otherwise ignored.
 *
 * @return A handle to the newly created message archive or null on failure.
 *
 * @see aug_createmar() and aug_releasemar().
 */

AUGMAR_API aug_mar*
aug_openmar_AIN(aug_mpool* mpool, const char* path, int flags, ...);

#endif /* AUGMAR_MAR_H */
