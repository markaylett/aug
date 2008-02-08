/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGBOOL_H
#define AUGBOOL_H

/* C++ enums are not guaranteed to have sizeof(int).  Therefore, int is used
   instead. */

enum aug_bool_ {
    AUG_FALSE,
    AUG_TRUE
};

typedef int aug_bool;

#endif /* AUGBOOL_H */
