/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_TYPES_H
#define AUGSRV_TYPES_H

#include "augutil/var.h"

#define AUG_RETNODAEMON (-2)

enum aug_option {

    /* The following options are required and should remain constant during
       the call to aug_main(). */

    AUG_OPTLONGNAME = 1,
    AUG_OPTSHORTNAME,
    AUG_OPTPROGRAM,
    AUG_OPTADMIN,

    /* The following option is optional (may be null) but should remain
       constant after the call to aug_service.config_(). */

    AUG_OPTCONFFILE,

    /* The following option is required and should remain constant during the
       call to aug_service.config_(). */

    AUG_OPTPIDFILE
};

struct aug_service {
    const char* (*getopt_)(const struct aug_var*, enum aug_option);
    int (*config_)(const struct aug_var*, const char*, int);
    int (*init_)(const struct aug_var*);
    int (*run_)(const struct aug_var*);
    struct aug_var arg_;
};

#endif /* AUGSRV_TYPES_H */
