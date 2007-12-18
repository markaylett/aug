/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_TYPES_H
#define AUGSRV_TYPES_H

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

struct aug_service {
    const char* (*getopt_)(void*, enum aug_option);
    int (*readconf_)(void*, const char*, int, int);
    int (*init_)(void*);
    int (*run_)(void*);
    void (*term_)(void*);
};

#endif /* AUGSRV_TYPES_H */
