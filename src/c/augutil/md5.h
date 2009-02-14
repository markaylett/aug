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
#ifndef AUGUTIL_BASE64_H
#define AUGUTIL_BASE64_H

/**
 * @file augutil/base64.h
 *
 * Base 64 encoder.
 */

#include "augutil/config.h"

#include "augsys/types.h"

#define AUG_MD5BASE64LEN 32

typedef char aug_md5base64_t[AUG_MD5BASE64LEN + 1];

struct aug_md5context {
    uint32_t buf[4];
    uint32_t bits[2];
    unsigned char in[64];
};

AUGUTIL_API void
aug_initmd5(struct aug_md5context* ctx);

AUGUTIL_API void
aug_appendmd5(struct aug_md5context* ctx, const unsigned char* buf,
              unsigned len);

AUGUTIL_API void
aug_finishmd5(unsigned char digest[16], struct aug_md5context* ctx);

AUGUTIL_API const char*
aug_md5base64(const unsigned char digest[16], aug_md5base64_t base64);

#endif /* AUGUTIL_MD5_H */
