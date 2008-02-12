/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGTYPE_H
#define AUGTYPE_H

#define AUG_SUCCESS  0
#define AUG_ESYSTEM  1

#define AUG_EASSERT  2
#define AUG_EAUTH    3
#define AUG_ECONFIG  4
#define AUG_EDOMAIN  5
#define AUG_EENDOF   6
#define AUG_EEXCEPT  7
#define AUG_EEXIST   8
#define AUG_EFORMAT  9
#define AUG_EINVAL   10
#define AUG_EIO      11
#define AUG_ELIMIT   12
#define AUG_ENULL    13
#define AUG_EMEMORY  14
#define AUG_EPARSE   15
#define AUG_EPERM    16
#define AUG_ERANGE   17
#define AUG_ESTATE   18
#define AUG_ESUPPORT 19
#define AUG_ETYPE    20
#define AUG_ETIMEOUT 21

/* C++ enums are not guaranteed to have sizeof(int).  Therefore, int is used
   instead. */

enum aug_bool_ {
    AUG_FALSE,
    AUG_TRUE
};

typedef int aug_bool;
typedef int aug_status;

#endif /* AUGTYPE_H */
