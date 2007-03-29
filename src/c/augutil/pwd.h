/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_PWD_H
#define AUGUTIL_PWD_H

#include "augutil/config.h"

#include "augsys/types.h"

#define AUG_MAXPASSWORD 128

AUGUTIL_API char*
aug_getpass(const char* prompt, char* buf, size_t len);

#endif /* AUGUTIL_PWD_H */
