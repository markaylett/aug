/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_TYPES_H
#define AUGSERV_TYPES_H

#include "augtypes.h"

enum aug_option {

    /**
     * The following options are required and should remain constant during
     * the call to aug_main().
     */

    AUG_OPTLONGNAME = 1,
    AUG_OPTSHORTNAME,
    AUG_OPTPROGRAM,
    AUG_OPTEMAIL,

    /**
     * The remaining options should remain constant after
     * aug_service.config_() has been called.
     */

    /**
     * Configuration file: optional (may be null).
     */

    AUG_OPTCONFFILE,

    /**
     * Pid file.
     */

    AUG_OPTPIDFILE
};

/**
 * Service callbacks.
 *
 * Called by the framework in the following order:
 *
 * @li readconf_()
 * @li init_()
 * @li run_()
 * @li term_()
 */

struct aug_service {
    const char* (*getopt_)(void*, int);
    aug_result (*readconf_)(void*, const char*, int, int);
    aug_result (*init_)(void*);
    aug_result (*run_)(void*);
    void (*term_)(void*);
};

#endif /* AUGSERV_TYPES_H */
