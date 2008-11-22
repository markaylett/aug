/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_CONTROL_H
#define AUGSERV_CONTROL_H

#include "augserv/config.h"

#include "augutil/event.h"

#include "augtypes.h"

struct aug_options;

AUGSERV_API aug_result
aug_start(const struct aug_options* options);

AUGSERV_API aug_result
aug_control(const struct aug_options* options, int event);

AUGSERV_API aug_result
aug_install(const struct aug_options* options);

AUGSERV_API aug_result
aug_uninstall(const struct aug_options* options);

#endif /* AUGSERV_CONTROL_H */
