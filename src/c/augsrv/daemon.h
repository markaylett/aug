/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_DAEMON_H
#define AUGSRV_DAEMON_H

#include "augsrv/config.h"

struct aug_service;

AUGSRV_API int
aug_daemonise(const struct aug_service* service);

#endif /* AUGSRV_DAEMON_H */
