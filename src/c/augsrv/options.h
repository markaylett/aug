/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_OPTIONS_H
#define AUGSRV_OPTIONS_H

#include "augsrv/config.h"

#define AUG_CMDNONE      0
#define AUG_CMDDEFAULT   1
#define AUG_CMDINSTALL   2
#define AUG_CMDRECONF    3
#define AUG_CMDSTART     4
#define AUG_CMDSTATUS    5
#define AUG_CMDSTOP      6
#define AUG_CMDTEST      7
#define AUG_CMDUNINSTALL 8

struct aug_service;

/* On success, returns one of the above commands. */

AUGSRV_API int
aug_readopts(const struct aug_service* service, int argc, char* argv[]);

#endif /* AUGSRV_OPTIONS_H */
