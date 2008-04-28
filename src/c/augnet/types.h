/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_TYPES_H
#define AUGNET_TYPES_H

#include "augsys/types.h"

#include "augabi.h"

struct aug_files;
struct aug_nbtype;

typedef struct aug_nbfiles_* aug_nbfiles_t;
typedef int (*aug_nbfilecb_t)(aug_object*, aug_md, unsigned short);

struct aug_nbfile {
    aug_nbfiles_t nbfiles_;
    aug_md md_;
    aug_nbfilecb_t cb_;
    const struct aug_fdtype* base_;
    const struct aug_nbtype* type_;
    void* ext_;
};

#endif /* AUGNET_TYPES_H */
