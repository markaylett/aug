/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_TYPES_H
#define AUGSRV_TYPES_H

#define AUG_FOREGROUND 1

#define AUG_SIGNONE   0
#define AUG_SIGRECONF 1
#define AUG_SIGSTATUS 2
#define AUG_SIGSTOP   3

/* Base value for user-defined signals. */

#define AUG_SIGUSER   4

typedef unsigned char aug_sig_t;

enum aug_option {
    AUG_OPTCONFFILE = 1,
    AUG_OPTPIDFILE
};

struct aug_service {
    int (*setopt_)(void*, enum aug_option, const char*);
    const char* (*getopt_)(void*, enum aug_option);
    int (*config_)(void*, int);
    int (*init_)(void*);
    int (*run_)(void*);
    const char* program_;
    const char* lname_;
    const char* sname_;
    const char* admin_;
    void* arg_;
};

#endif /* AUGSRV_TYPES_H */
