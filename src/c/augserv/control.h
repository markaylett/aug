/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_CONTROL_H
#define AUGSERV_CONTROL_H

#include "augserv/config.h"

#include "augutil/event.h"

#include "augtypes.h"

AUGSERV_API aug_result
aug_start(void);

AUGSERV_API aug_result
aug_control(int event);

AUGSERV_API aug_result
aug_install(void);

AUGSERV_API aug_result
aug_uninstall(void);

#endif /* AUGSERV_CONTROL_H */
