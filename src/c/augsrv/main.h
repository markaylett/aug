/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_MAIN_H
#define AUGSRV_MAIN_H

#include "augsrv/config.h"

struct aug_server;

/**
   On Windows, the Service Manager calls the service entry point on a separate
   thread - automatic variables on the main thread's stack will not be visible
   from the service thread.  A shallow copy of the server structure is,
   therefore, performed by aug_main().
*/

AUGSRV_API int
aug_main(int argc, char* argv[], const struct aug_server* server, void* arg);

#endif /* AUGSRV_MAIN_H */
