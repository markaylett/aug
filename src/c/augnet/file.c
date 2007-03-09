/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/file.h"

static const char rcsid[] = "$Id$";

#include "augutil/var.h"

#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/lock.h"

#include <stdlib.h>

struct aug_file_ {
    AUG_ENTRY(aug_file_);
    int fd_;
    aug_filecb_t cb_;
    struct aug_var var_;
};

static struct aug_files free_ = AUG_HEAD_INITIALIZER(free_);
AUG_ALLOCATOR(allocate_, &free_, aug_file_, 64)

AUGNET_API int
aug_destroyfiles(struct aug_files* files)
{
    struct aug_file_* it;
    AUG_FOREACH(it, files)
        aug_destroyvar(&it->var_);

    if (!AUG_EMPTY(files)) {

        aug_lock();
        AUG_CONCAT(&free_, files);
        aug_unlock();
    }
    return 0;
}

AUGNET_API int
aug_insertfile(struct aug_files* files, int fd, aug_filecb_t cb,
               const struct aug_var* var)
{
    struct aug_file_* file;

    aug_lock();
    if (!(file = allocate_())) {
        aug_unlock();
        return -1;
    }
    aug_unlock();

    file->fd_ = fd;
    file->cb_ = cb;
    aug_setvar(&file->var_, var);

    AUG_INSERT_TAIL(files, file);
    return 0;
}

AUGNET_API int
aug_removefile(struct aug_files* files, int fd)
{
    struct aug_file_* it;

    AUG_FOREACH(it, files)
        if (it->fd_ == fd)
            break;

    if (!it) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                       AUG_MSG("no file for descriptor '%d'"), (int)fd);
        return -1;
    }

    AUG_REMOVE(files, it, aug_file_);

    aug_destroyvar(&it->var_);
    aug_lock();
    AUG_INSERT_TAIL(&free_, it);
    aug_unlock();

    return 0;
}

AUGNET_API int
aug_foreachfile(struct aug_files* files)
{
    struct aug_file_* it, ** prev;
    struct aug_files tail;
    AUG_INIT(&tail);

    prev = &AUG_FIRST(files);
    while ((it = *prev)) {

        if (!(it->cb_(it->fd_, &it->var_, &tail))) {

            AUG_REMOVE_PREVPTR(it, prev, files);

            aug_destroyvar(&it->var_);
            aug_lock();
            AUG_INSERT_TAIL(&free_, it);
            aug_unlock();

        } else
            prev = &AUG_NEXT(it);
    }

    AUG_CONCAT(files, &tail);

    /* Rotate files to promote fairness. */

    if (!(it = AUG_FIRST(files))) {
        AUG_REMOVE_HEAD(files);
        AUG_INSERT_TAIL(files, it);
    }
    return 0;
}
