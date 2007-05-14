/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   TODO
   \file file.h
 */

#ifndef AUGMAR_FILE_H_
#define AUGMAR_FILE_H_

#include "augmar/config.h"
#include "augmar/types.h"

AUG_EXTERN int
aug_closefile_(int fd);

AUG_EXTERN int
aug_openfile_(const char* path, int flags, mode_t mode);

AUG_EXTERN int
aug_extendfile_(int fd, unsigned size);

AUG_EXTERN int
aug_syncfile_(int fd);

AUG_EXTERN int
aug_truncatefile_(int fd, off_t size);

#endif /* AUGMAR_FILE_H_ */
