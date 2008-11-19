/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_TYPES_H
#define AUGSERV_TYPES_H

enum aug_option {

    /**
     * The following options are required and must remain constant during the
     * call to aug_main().
     */

    AUG_OPTLONGNAME = 1,
    AUG_OPTSHORTNAME,
    AUG_OPTPROGRAM,
    AUG_OPTEMAIL,

    /**
     * The remaining options must not change after aug_readservconf() has been
     * called.
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

#endif /* AUGSERV_TYPES_H */
