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
#ifndef AUGEXT_VAR_H
#define AUGEXT_VAR_H

#include "augconfig.h"

#include "augext/blob.h"
#include "augext/seq.h"

#include <time.h>

enum aug_vartype {
    AUG_VTNULL,
    AUG_VTINT,
    AUG_VTINT32,
    AUG_VTINT64,
    AUG_VTBOOL,
    AUG_VTDOUBLE,
    AUG_VTOBJECT,
    AUG_VTBLOB,
    AUG_VTSEQ
};

struct aug_var {
    enum aug_vartype type_;
    union {
        int int_;
        int32_t i32_;
        int64_t i64_;
        double dbl_;
        aug_object* ob_;
        aug_blob* blob_;
        aug_seq* seq_;
    } u_;
};

#define AUG_VARINT(x) ((x)->u_.int_)
#define AUG_VARINT32(x) ((x)->u_.i32_)
#define AUG_VARINT64(x) ((x)->u_.i64_)
#define AUG_VARBOOL(x) ((x)->u_.int_)
#define AUG_VARDOUBLE(x) ((x)->u_.dbl_)
#define AUG_VAROBJECT(x) ((x)->u_.ob_)
#define AUG_VARBLOB(x) ((x)->u_.blob_)
#define AUG_VARSEQ(x) ((x)->u_.seq_)

#define AUG_RETAINVAR(x) \
do { \
    if (AUG_VTOBJECT <= (x)->type_ && (x)->u_.ob_) \
        aug_retain((x)->u_.ob_); \
} while (0)

#define AUG_RELEASEVAR(x) \
do { \
    if (AUG_VTOBJECT <= (x)->type_) { \
        if ((x)->u_.ob_) { \
            aug_release((x)->u_.ob_); \
            (x)->u_.ob_ = NULL; \
        } \
        (x)->type_ = AUG_VTNULL; \
    } \
} while (0)

#endif /* AUGEXT_VAR_H */
