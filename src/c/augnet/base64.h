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
#ifndef AUGNET_BASE64_H
#define AUGNET_BASE64_H

/**
 * @file augnet/base64.h
 *
 * Base 64 encoder and decoder.
 */

#include "augnet/config.h"

#include "augsys/types.h"

#include "augext/mpool.h"

#include "augabi.h"
#include "augtypes.h"

typedef aug_result (*aug_base64cb_t)(const char*, size_t, aug_object*);

typedef struct aug_base64_* aug_base64_t;

enum aug_base64mode {
	AUG_DECODE64,
	AUG_ENCODE64
};

AUGNET_API aug_base64_t
aug_createbase64(aug_mpool* mpool, int mode, aug_base64cb_t cb,
                 aug_object* ob);

AUGNET_API void
aug_destroybase64(aug_base64_t base64);

AUGNET_API aug_result
aug_appendbase64(aug_base64_t base64, const char* src, size_t len);

AUGNET_API aug_result
aug_finishbase64(aug_base64_t base64);

#endif /* AUGNET_BASE64_H */
