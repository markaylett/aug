/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_MAIN_H
#define AUGSRV_MAIN_H

#include "augsrv/config.h"

struct aug_service;

AUGSRV_API void
aug_main(const struct aug_service* service, int argc, char* argv[]);

#endif /* AUGSRV_MAIN_H */
