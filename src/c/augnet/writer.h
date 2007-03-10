/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_WRITER_H
#define AUGNET_WRITER_H

#include "augnet/config.h"

#include "augsys/types.h"

struct aug_var;

typedef struct aug_writer_* aug_writer_t;

AUGNET_API aug_writer_t
aug_createwriter(void);

AUGNET_API int
aug_destroywriter(aug_writer_t writer);

AUGNET_API int
aug_appendbuf(aug_writer_t writer, const struct aug_var* var);

AUGNET_API int
aug_emptybuf(aug_writer_t writer);

AUGNET_API ssize_t
aug_writesome(aug_writer_t writer, int fd);

#endif /* AUGNET_WRITER_H */
