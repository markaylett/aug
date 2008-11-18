/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_H
#define AUGSERV_H

/**
 * @file augserv.h
 *
 * Support for process daemonisation and NT services.
 *
 * On Windows, service applications run as NT services; on Linux, they run as
 * daemonised processes.
 */

#include "augserv/base.h"
#include "augserv/control.h"
#include "augserv/daemon.h"
#include "augserv/log.h"
#include "augserv/main.h"
#include "augserv/options.h"
#include "augserv/signal.h"
#include "augserv/types.h"

#endif /* AUG_SERV */
