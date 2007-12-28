/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGOB_TYPES_H
#define AUGOB_TYPES_H

#include "augob/blob.h"

#include <time.h>

#if !defined(AUG_INTTYPES)
# define AUG_INTTYPES
# if !defined(_MSC_VER)
#  include <inttypes.h>
# else /* _MSC_VER */

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;

typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

# endif /* _MSC_VER */
#endif /* AUG_INTTYPES */

enum aug_vartype {
    AUG_VTNULL,
    AUG_VTINT,
    AUG_VTINT32,
    AUG_VTINT64,
    AUG_VTBOOL,
    AUG_VTDOUBLE,
    AUG_VTTIME,
    AUG_VTSTRING,
    AUG_VTOBJECT,
};

struct aug_var {
    enum aug_vartype type_;
    union {
        int int_;
        int32_t i32_;
        int64_t i64_;
        double dbl_;
        time_t utc_;
        aug_blob* str_;
        aub_object* ob_;
    } u_;
};

#define AUG_VARINT(x) ((x)->u_.int_)
#define AUG_VARINT32(x) ((x)->u_.i32_)
#define AUG_VARINT64(x) ((x)->u_.i64_)
#define AUG_VARBOOL(x) ((x)->u_.int_)
#define AUG_VARDOUBLE(x) ((x)->u_.dbl_)
#define AUG_VARTIME(x) ((x)->u_.utc_)
#define AUG_VARSTRING(x) ((x)->u_.str_)
#define AUG_VAROBJECT(x) ((x)->u_.ob_)

#define AUG_RELEASEVAR(x) \
do { \
    if (AUG_VTOBJECT == (x)->type_ || AUG_VTSTRING == (x)->type_) { \
        if ((x)->u_.ob_) { \
            aub_release((x)->u_.ob_); \
            (x)->u_.ob_ = NULL; \
        } \
        (x)->type_ = AUG_VTNULL; \
    } \
} while (0)

#endif /* AUGOB_TYPES_H */
