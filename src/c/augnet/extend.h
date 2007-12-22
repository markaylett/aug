/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_EXTEND_H
#define AUGNET_EXTEND_H

/**
 * @file augnet/extend.h
 *
 * File type extensions.
 */

#include "augnet/file.h"
#include "augnet/types.h"

#include "augsys/muxer.h"

struct aug_nbfiles_ {
    aug_muxer_t muxer_;
    struct aug_files files_;
    int nowait_;
};

struct aug_nbtype {
    int (*filecb_)(aug_object*, struct aug_nbfile*);
    int (*seteventmask_)(struct aug_nbfile*, unsigned short);
    int (*eventmask_)(struct aug_nbfile*);
    int (*events_)(struct aug_nbfile*);
    int (*shutdown_)(struct aug_nbfile*);
};

/**
 * Thread-safe.
 */

AUGNET_API const struct aug_nbfile*
aug_setnbfile(int fd, const struct aug_nbfile* nbfile);

/**
 * Thread-safe.
 */

AUGNET_API struct aug_nbfile*
aug_getnbfile(int fd, struct aug_nbfile* nbfile);

/**
 * Thread-safe.
 */

AUGNET_API struct aug_nbfile*
aug_resetnbfile(int fd, struct aug_nbfile* nbfile);

#endif /* AUGNET_EXTEND_H */
