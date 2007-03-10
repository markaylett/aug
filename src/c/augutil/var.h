/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_VAR_H
#define AUGUTIL_VAR_H

#include "augutil/config.h"

#include "augsys/types.h"

struct aug_vartype {
    int (*destroy_)(void*);
    const void* (*buf_)(void*, size_t*);
};

struct aug_var {
    const struct aug_vartype* type_;
    void* arg_;
};

#define AUG_VARNULL { NULL, NULL }

AUGUTIL_API int
aug_destroyvar(const struct aug_var* var);

AUGUTIL_API void
aug_setvar(struct aug_var* dst, const struct aug_var* src);

AUGUTIL_API const void*
aug_varbuf(const struct aug_var* var, size_t* size);

#endif /* AUGUTIL_VAR_H */
