/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_FILE_H
#define AUGNET_FILE_H

#include "augnet/config.h"

#include "augutil/list.h"

struct aug_var;

/**
   #AUG_INIT should be used to initialise the aug_files structure.
 */

struct aug_file_;
AUG_HEAD(aug_files, aug_file_);

/**
   The callback function has a boolean return value: returning false removes
   the file.
*/
typedef int (*aug_filecb_t)(const struct aug_var*, int, struct aug_files*);

AUGNET_API int
aug_destroyfiles(struct aug_files* files);

/**
   If aug_insertfile() succeeds, aug_destroyvar() will be called when the file is
   removed.
*/

AUGNET_API int
aug_insertfile(struct aug_files* files, int fd, aug_filecb_t cb,
               const struct aug_var* var);

AUGNET_API int
aug_removefile(struct aug_files* files, int fd);

AUGNET_API int
aug_foreachfile(struct aug_files* files);

/**
   \return a pointer to the aug_var structure associated with the file, or
   NULL if the file does not exist.
 */

AUGNET_API const struct aug_var*
aug_filevar(struct aug_files* files, int fd);

#endif /* AUGNET_FILE_H */
