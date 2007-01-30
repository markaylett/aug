/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_FILE_H
#define AUGNET_FILE_H

#include "augnet/config.h"

#include "augutil/list.h"

struct aug_var;

struct aug_file_;
AUG_HEAD(aug_files, aug_file_);

/**
   The callback function has a boolean return value: returning false removes
   the file.
*/
typedef int (*aug_filecb_t)(int, const struct aug_var*, struct aug_files*);

AUGNET_API int
aug_freefiles(struct aug_files* files);

/**
   If aug_insertfile() succeeds, aug_freevar() will be called when the file is
   removed.
*/

AUGNET_API int
aug_insertfile(struct aug_files* files, int fd, aug_filecb_t cb,
               const struct aug_var* arg);

AUGNET_API int
aug_removefile(struct aug_files* files, int fd);

AUGNET_API int
aug_foreachfile(struct aug_files* files);

#endif /* AUGNET_FILE_H */
