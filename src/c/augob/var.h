/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOB_VAR_H
#define AUGOB_VAR_H

#include "augconfig.h"

#include "augob/blob.h"
#include "augob/seqob.h"

#include <time.h>

enum aug_vartype {
    AUG_VTNULL,
    AUG_VTINT,
    AUG_VTINT32,
    AUG_VTINT64,
    AUG_VTBOOL,
    AUG_VTDOUBLE,
    AUG_VTTIME,
    AUG_VTOBJECT,
    AUG_VTSTRING,
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
        time_t utc_;
        aub_object* ob_;
        aug_blob* blob_;
        aug_seqob* seq_;
    } u_;
};

#define AUG_VARINT(x) ((x)->u_.int_)
#define AUG_VARINT32(x) ((x)->u_.i32_)
#define AUG_VARINT64(x) ((x)->u_.i64_)
#define AUG_VARBOOL(x) ((x)->u_.int_)
#define AUG_VARDOUBLE(x) ((x)->u_.dbl_)
#define AUG_VARTIME(x) ((x)->u_.utc_)
#define AUG_VAROBJECT(x) ((x)->u_.ob_)
#define AUG_VARSTRING(x) ((x)->u_.blob_)
#define AUG_VARBLOB(x) ((x)->u_.blob_)
#define AUG_VARSEQ(x) ((x)->u_.seq_)

#define AUG_RETAINVAR(x) \
do { \
    if (AUG_VTOBJECT <= (x)->type_ && (x)->u_.ob_) \
        aub_retain((x)->u_.ob_); \
} while (0)

#define AUG_RELEASEVAR(x) \
do { \
    if (AUG_VTOBJECT <= (x)->type_) { \
        if ((x)->u_.ob_) { \
            aub_release((x)->u_.ob_); \
            (x)->u_.ob_ = NULL; \
        } \
        (x)->type_ = AUG_VTNULL; \
    } \
} while (0)

#endif /* AUGOB_VAR_H */
