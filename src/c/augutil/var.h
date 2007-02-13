/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_VAR_H
#define AUGUTIL_VAR_H

#include "augutil/config.h"

enum aug_vartype {
    AUG_VTNULL,
    AUG_VTLONG,
    AUG_VTPTR
};

struct aug_var {
    enum aug_vartype type_;
    union {
        long long_;
        void* ptr_;
    } u_;
    void (*destroy_)();
};

#define AUG_VARNULL { AUG_VTNULL, { 0 }, NULL }

/**
   "v" may be null.
*/

AUGUTIL_API const struct aug_var*
aug_destroyvar(const struct aug_var* v);

AUGUTIL_API struct aug_var*
aug_clearvar(struct aug_var* v, void (*fn)(void));

AUGUTIL_API struct aug_var*
aug_setvar(struct aug_var* v, const struct aug_var* w);

AUGUTIL_API struct aug_var*
aug_setvarl(struct aug_var* v, long l, void (*fn)(long));

AUGUTIL_API struct aug_var*
aug_setvarp(struct aug_var* v, void* p, void (*fn)(void*));

AUGUTIL_API long
aug_getvarl(const struct aug_var* v);

AUGUTIL_API void*
aug_getvarp(const struct aug_var* v);

AUGUTIL_API int
aug_equalvar(const struct aug_var* v, const struct aug_var* w);

AUGUTIL_API int
aug_isnull(const struct aug_var* v);

#endif /* AUGUTIL_VAR_H */
