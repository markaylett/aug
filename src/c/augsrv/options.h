/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_OPTIONS_H
#define AUGSRV_OPTIONS_H

#include "augsrv/config.h"

#include "augsys/limits.h" /* AUG_PATH_MAX */

enum aug_command {

    AUG_CMDDEFAULT,
    AUG_CMDEXIT,
    AUG_CMDINSTALL,
    AUG_CMDRECONF,
    AUG_CMDSTART,
    AUG_CMDSTATUS,
    AUG_CMDSTOP,
    AUG_CMDTEST,
    AUG_CMDUNINSTALL
};

struct aug_options {
    char conffile_[AUG_PATH_MAX + 1];
    int batch_;
    enum aug_command command_;
};

AUGSRV_API int
aug_readopts(struct aug_options* options, int argc, char* argv[]);

#endif /* AUGSRV_OPTIONS_H */
