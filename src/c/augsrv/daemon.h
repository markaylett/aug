/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_DAEMON_H
#define AUGSRV_DAEMON_H

#include "augsrv/config.h"

/**
 * Called from aug_main().
 *
 * @return On Windows, #AUG_RETNONE if the service has not been installed.
 */

AUGSRV_API int
aug_daemonise(void);

#endif /* AUGSRV_DAEMON_H */
