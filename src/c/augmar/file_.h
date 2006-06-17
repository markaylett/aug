/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file file.h
 * \brief TODO
 */

#ifndef AUGMAR_FILE_H_
#define AUGMAR_FILE_H_

#include "augmar/config.h"
#include "augmar/types.h"

AUGMAR_EXTERN int
aug_closefile_(int fd);

AUGMAR_EXTERN int
aug_openfile_(const char* path, int flags, mode_t mode);

AUGMAR_EXTERN int
aug_extendfile_(int fd, size_t size);

AUGMAR_EXTERN int
aug_syncfile_(int fd);

AUGMAR_EXTERN int
aug_truncatefile_(int fd, off_t size);

#endif /* AUGMAR_FILE_H_ */
