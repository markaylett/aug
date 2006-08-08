/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_CONN_H
#define AUGNET_CONN_H

#include "augnet/config.h"

#include "augutil/list.h"

struct aug_var;

struct aug_conn_;
AUG_HEAD(aug_conns, aug_conn_);

/** The callback function has a boolean return value: returning false removes
    the connection. */

typedef int (*aug_conncb_t)(const struct aug_var*, int, struct aug_conns*);

AUGNET_API int
aug_freeconns(struct aug_conns* conns);

AUGNET_API int
aug_insertconn(struct aug_conns* conns, int fd, aug_conncb_t cb,
               const struct aug_var* arg);

AUGNET_API int
aug_removeconn(struct aug_conns* conns, int fd);

AUGNET_API int
aug_processconns(struct aug_conns* conns);

#endif /* AUGNET_CONN_H */
