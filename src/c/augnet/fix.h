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
#ifndef AUGNET_FIX_H
#define AUGNET_FIX_H

/**
 * @file augnet/fix.h
 *
 * FIX parser.
 */

#include "augnet/config.h"

#include "augext/mpool.h"
#include "augext/stream.h"

#define AUG_FIXVERLEN  7  /* FIX.x.y */

typedef char aug_fixver_t[AUG_FIXVERLEN + 1];
typedef unsigned aug_fixtag_t;

struct aug_fixfield_ {
    aug_fixtag_t tag_;
    const char* value_;
    aug_len size_;
};

struct aug_fixstd_ {
    aug_fixver_t fixver_;
    const char* body_;
    size_t size_;
};

typedef void (*aug_fixcb_t)(const char*, size_t, aug_object*);

typedef struct aug_fixstream_* aug_fixstream_t;

/**
 * If aug_createfixstream() succeeds, aug_release() will be called from
 * aug_destroyfixstream().
 */

AUGNET_API aug_fixstream_t
aug_createfixstream(aug_mpool* mpool, size_t size, aug_fixcb_t cb,
                    aug_object* ob);

AUGNET_API void
aug_destroyfixstream(aug_fixstream_t stream);

AUGNET_API aug_rsize
aug_readfix(aug_fixstream_t stream, aug_stream* src, size_t size);

AUGNET_API aug_result
aug_finishfix(aug_fixstream_t stream);

AUGNET_API aug_result
aug_checkfix(struct aug_fixstd_* fixstd, const char* buf, size_t size);

AUGNET_API aug_len
aug_checksum(const char* buf, size_t size);

AUGNET_API aug_rsize
aug_fixfield(struct aug_fixfield_* field, const char* buf, size_t size);

#endif /* AUGNET_FIX_H */
