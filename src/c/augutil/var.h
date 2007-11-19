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

/**
   Set destination var from source.

   \param dst Destination var.

   \param src Optional source var.  If NULL, both "type_" and "arg_" of "dst"
   will be set to NULL.

   \return Destination var.
 */

AUGUTIL_API struct aug_var*
aug_setvar(struct aug_var* dst, const struct aug_var* src);

/**
   Translate to buffer using "buf_" function of "type_".

   \return Result of applying function to "arg_", or NULL if no "buf_"
   function exists.
 */

AUGUTIL_API const void*
aug_varbuf(const struct aug_var* var, size_t* size);

/**
   Pack integer into var.
 */

AUGUTIL_API void*
aug_itop(int i);

/**
   Unpack integer from var.
*/

AUGUTIL_API int
aug_ptoi(void* p);

#endif /* AUGUTIL_VAR_H */
