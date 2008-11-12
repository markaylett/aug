/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_H
#define AUGSRV_H

/**
 * @file augsrv.h
 *
 * Support for process daemonisation and NT services.
 *
 * On Windows, service applications run as NT services; on Linux, they run as
 * daemonised processes.
 */

#include "augsrv/base.h"
#include "augsrv/control.h"
#include "augsrv/daemon.h"
#include "augsrv/log.h"
#include "augsrv/main.h"
#include "augsrv/options.h"
#include "augsrv/signal.h"
#include "augsrv/types.h"

#endif /* AUG_SRV */
