/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/var.h"

static const char rcsid[] = "$Id:$";

#include <stddef.h> /* NULL */

AUGUTIL_API struct aug_var*
aug_clearvar(struct aug_var* v)
{
    v->type_ = AUG_VTNULL;
    v->u_.ptr_ = NULL;
    return v;
}

AUGUTIL_API struct aug_var*
aug_setvarl(struct aug_var* v, long l)
{
    v->type_ = AUG_VTLONG;
    v->u_.long_ = l;
    return v;
}

AUGUTIL_API struct aug_var*
aug_setvarp(struct aug_var* v, void* p)
{
    v->type_ = p ? AUG_VTPTR : AUG_VTNULL;
    v->u_.ptr_ = p;
    return v;
}

AUGUTIL_API struct aug_var*
aug_setvar(struct aug_var* v, const struct aug_var* w)
{
    if (w) {
        if (AUG_VTLONG == (v->type_ = w->type_))
            v->u_.long_ = w->u_.long_;
        else
            v->u_.ptr_ = w->u_.ptr_;
    } else {
        v->type_ = AUG_VTNULL;
        v->u_.ptr_ = NULL;
    }
    return v;
}

AUGUTIL_API int
aug_isnull(const struct aug_var* v)
{
    return !v || AUG_VTNULL == v->type_
        || (AUG_VTPTR == v->type_ && !v->u_.ptr_);
}

AUGUTIL_API long
aug_varl(const struct aug_var* v)
{
    return v && AUG_VTLONG == v->type_ ? v->u_.long_ : 0;
}

AUGUTIL_API void*
aug_varp(const struct aug_var* v)
{
    return v && AUG_VTPTR == v->type_ ? v->u_.ptr_ : NULL;
}
