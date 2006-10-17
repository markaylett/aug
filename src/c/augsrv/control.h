/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_CONTROL_H
#define AUGSRV_CONTROL_H

#include "augsrv/config.h"

#include "augutil/event.h"

AUGSRV_API int
aug_start(void);

AUGSRV_API int
aug_control(int event);

AUGSRV_API int
aug_install(void);

AUGSRV_API int
aug_uninstall(void);

#endif /* AUGSRV_CONTROL_H */
