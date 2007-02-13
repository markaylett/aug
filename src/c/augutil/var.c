/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/var.h"

static const char rcsid[] = "$Id$";

#include <stddef.h> /* NULL */

AUGUTIL_API const struct aug_var*
aug_destroyvar(const struct aug_var* v)
{
    if (!v)
        return NULL;

    if (v->destroy_) {
        switch (v->type_) {
        case AUG_VTNULL:
            v->destroy_();
            break;
        case AUG_VTLONG:
            v->destroy_(v->u_.long_);
            break;
        case AUG_VTPTR:
            v->destroy_(v->u_.ptr_);
            break;
        }
    }
    return v;
}

AUGUTIL_API struct aug_var*
aug_clearvar(struct aug_var* v, void (*fn)(void))
{
    v->type_ = AUG_VTNULL;
    v->u_.ptr_ = NULL;
    v->destroy_ = fn;
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
        v->destroy_ = w->destroy_;
    } else {
        v->type_ = AUG_VTNULL;
        v->u_.ptr_ = NULL;
        v->destroy_ = NULL;
    }
    return v;
}

AUGUTIL_API struct aug_var*
aug_setvarl(struct aug_var* v, long l, void (*fn)(long))
{
    v->type_ = AUG_VTLONG;
    v->u_.long_ = l;
    v->destroy_ = fn;
    return v;
}

AUGUTIL_API struct aug_var*
aug_setvarp(struct aug_var* v, void* p, void (*fn)(void*))
{
    v->type_ = AUG_VTPTR;
    v->u_.ptr_ = p;
    v->destroy_ = fn;
    return v;
}

AUGUTIL_API long
aug_getvarl(const struct aug_var* v)
{
    return v && AUG_VTLONG == v->type_ ? v->u_.long_ : 0;
}

AUGUTIL_API void*
aug_getvarp(const struct aug_var* v)
{
    return v && AUG_VTPTR == v->type_ ? v->u_.ptr_ : NULL;
}

AUGUTIL_API int
aug_equalvar(const struct aug_var* v, const struct aug_var* w)
{
    if (AUG_VTLONG == v->type_)
        return AUG_VTLONG == w->type_
            && v->u_.long_ == w->u_.long_;

    return v->u_.ptr_ == w->u_.ptr_;
}

AUGUTIL_API int
aug_isnull(const struct aug_var* v)
{
    return !v || AUG_VTNULL == v->type_
        || (AUG_VTPTR == v->type_ && !v->u_.ptr_);
}
