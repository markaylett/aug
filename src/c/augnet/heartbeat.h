/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_HEARTBEAT_H
#define AUGNET_HEARTBEAT_H

#include "augnet/config.h"

#define AUG_HBNAMELEN 64

struct aug_heartbeat {
    unsigned head_;
    char name_[AUG_HBNAMELEN + 1];
    unsigned seq_, state_, load_, hbint_, msg_;
};

AUGNET_API char*
aug_packheartbeat(char* buf, const struct aug_heartbeat* hb);

AUGNET_API struct aug_heartbeat*
aug_unpackheartbeat(struct aug_heartbeat* hb, const char* buf);

#endif /* AUGNET_HEARTBEAT_H */
