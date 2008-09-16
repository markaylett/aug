/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_CONTROL_H
#define AUGSRV_CONTROL_H

#include "augsrv/config.h"

#include "augutil/event.h"

#include "augtypes.h"

AUGSRV_API aug_result
aug_start(void);

AUGSRV_API aug_result
aug_control(int event);

AUGSRV_API aug_result
aug_install(void);

AUGSRV_API aug_result
aug_uninstall(void);

#endif /* AUGSRV_CONTROL_H */
