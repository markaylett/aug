/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_FILE_H
#define AUGNET_FILE_H

#include "augnet/config.h"

#include "augutil/list.h"

#include "augobj.h"

/**
   #AUG_INIT should be used to initialise the aug_files structure.
 */

struct aug_file_;
AUG_HEAD(aug_files, aug_file_);

/**
   Callback function has a boolean return value: returning false removes the
   file.
*/
typedef int (*aug_filecb_t)(aug_object*, int);

/**
   Destroy each file in the list.

   aug_decref() is called for each ob associated with each file.

   \param files List of files to be destroyed.
 */

AUGNET_API int
aug_destroyfiles(struct aug_files* files);

/**
   Insert file into file list.

   If aug_insertfile() succeeds, aug_incref() will be called on "ob".

   \param files The file list.
   \param fd File descriptor.
   \param cb Callback to be called during aug_foreachfile() iterations.
   \param ob Optional object variable.
*/

AUGNET_API int
aug_insertfile(struct aug_files* files, int fd, aug_filecb_t cb,
               aug_object* ob);

/**
   Remove file from list.

   \param files The file list.
   \param fd Descriptor associated with file to be removed.
*/

AUGNET_API int
aug_removefile(struct aug_files* files, int fd);

/**
   Iterate list of files.

   The callback function is called for each file in the list.

   \param files The file list.
*/

AUGNET_API int
aug_foreachfile(struct aug_files* files);

#endif /* AUGNET_FILE_H */
