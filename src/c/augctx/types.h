/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_TYPES_H
#define AUGCTX_TYPES_H

enum aug_loglevel {
    AUG_LOGCRIT,
    AUG_LOGERROR,
    AUG_LOGWARN,
    AUG_LOGNOTICE,
    AUG_LOGINFO,
    AUG_LOGDEBUG0
};

struct aug_errinfo {
    char file_[512];
    int line_;
    char src_[32];
    int num_;
    char desc_[512];
};

#endif /* AUGCTX_TYPES_H */
