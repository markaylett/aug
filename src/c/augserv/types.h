/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_TYPES_H
#define AUGSERV_TYPES_H

#include "augext/task.h"

enum aug_option {

    /**
     * The following options are required constants.
     */

    AUG_OPTLONGNAME = 1,
    AUG_OPTSHORTNAME,
    AUG_OPTPROGRAM,
    AUG_OPTEMAIL,

    /**
     * Once aug_readservconf() has been called, the following options may also
     * be requested.  Once requested, they should remain constant thereafter.
     */

    /**
     * Pid file.
     */

    AUG_OPTPIDFILE
};

struct aug_serv {
    const char* (*getopt_)(int);
    aug_result (*readconf_)(const char*, aug_bool, aug_bool);
    aug_task* (*create_)(void);
};

#endif /* AUGSERV_TYPES_H */
