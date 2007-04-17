/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/ssl.h"
#include "augsys/defs.h"

AUG_RCSID("$Id:$");

#include "augsys/errinfo.h"

AUGNET_API int
aug_startsslclient(int fd, void* ctx)
{
    aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                   AUG_MSG("aug_startsslclient() not supported"));
    return -1;
}

AUGNET_API int
aug_startsslserver(int fd, void* ctx)
{
    aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                   AUG_MSG("aug_startsslclient() not supported"));
    return -1;
}
