/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UTILITY_H
#define AUGSYS_UTILITY_H

#include "augsys/config.h"
#include "augsys/types.h"

AUGSYS_API int
aug_filesize(int fd, size_t* size);

AUGSYS_API int
aug_setnonblock(int fd, int on);

#endif /* AUGSYS_UTILITY_H */
