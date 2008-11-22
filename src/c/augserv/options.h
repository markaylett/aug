/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_OPTIONS_H
#define AUGSERV_OPTIONS_H

#include "augserv/config.h"

#include "augsys/limits.h" /* AUG_PATH_MAX */

#include "augtypes.h"

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
    aug_bool batch_;
    enum aug_command command_;
};

AUGSERV_API aug_result
aug_readopts(struct aug_options* options, int argc, char* argv[]);

#define AUG_CONFFILE(x) (*(x)->conffile_ ? (x)->conffile_ : NULL)

#endif /* AUGSERV_OPTIONS_H */
