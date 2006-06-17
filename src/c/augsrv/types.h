/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_TYPES_H
#define AUGSRV_TYPES_H

#define AUG_FOREGROUND 1

enum aug_option {
    AUG_OPTCONFFILE = 1,
    AUG_OPTPIDFILE
};

struct aug_service {
    const char* (*getopt_)(void*, enum aug_option);
    int (*config_)(void*, const char*, int);
    int (*init_)(void*);
    int (*run_)(void*);
    const char* program_;
    const char* lname_;
    const char* sname_;
    const char* admin_;
    void* arg_;
};

#endif /* AUGSRV_TYPES_H */
