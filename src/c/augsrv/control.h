/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_CONTROL_H
#define AUGSRV_CONTROL_H

#include "augsrv/config.h"

#include "augutil/event.h"

struct aug_service;

AUGSRV_API int
aug_start(const struct aug_service* service);

AUGSRV_API int
aug_control(const struct aug_service* service, int event);

AUGSRV_API int
aug_install(const struct aug_service* service);

AUGSRV_API int
aug_uninstall(const struct aug_service* service);

#endif /* AUGSRV_CONTROL_H */
