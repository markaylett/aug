/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/var.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include <stddef.h> /* NULL */

AUGUTIL_API int
aug_destroyvar(const struct aug_var* v)
{
    return v && v->type_ && v->type_->destroy_
        ? v->type_->destroy_(v->arg_) : 0;
}

AUGUTIL_API void
aug_setvar(struct aug_var* dst, const struct aug_var* src)
{
    if (src)
        *dst = *src;
    else {
        dst->type_ = NULL;
        dst->arg_ = NULL;
    }
}

AUGUTIL_API const void*
aug_varbuf(const struct aug_var* v, size_t* size)
{
    if (v && v->type_ && v->type_->buf_)
        return v->type_->buf_(v->arg_, size);

    if (size)
        *size = 0;
    return 0;
}

AUGUTIL_API void*
aug_itop(int i)
{
	union {
		int i_;
		void* p_;
	} u;
    u.i_ = i;
    return u.p_;
}

AUGUTIL_API int
aug_ptoi(void* p)
{
	union {
		int i_;
		void* p_;
	} u;
    u.p_ = p;
    return u.i_;
}
