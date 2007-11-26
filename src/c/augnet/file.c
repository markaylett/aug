/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/file.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augutil/var.h"

#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/lock.h"

#include <stdlib.h>

struct aug_file_ {
    AUG_ENTRY(aug_file_);
    int trash_, fd_;
    aug_filecb_t cb_;
    aug_object* user_;
};

static struct aug_files free_ = AUG_HEAD_INITIALIZER(free_);
AUG_ALLOCATOR(allocate_, &free_, aug_file_, 64)

/* When added to the front of a list, this marker indicates that the list is
   locked. */

static struct aug_file_ lockfile_ = { { 0 }, 0, -1, NULL, NULL };
#define LOCKFILE_ (&lockfile_)

static void
lock_(struct aug_files* files)
{
    /* Added lock marker to front. */

    AUG_INSERT_HEAD(files, LOCKFILE_);
}

static void
unlock_(struct aug_files* files)
{
    struct aug_file_* it, ** prev;
    struct aug_files trash;
    AUG_INIT(&trash);

    /* Remove lock marker from front. */

    AUG_REMOVE_HEAD(files);

    /* Moved all trashed items to trash list. */

    prev = &AUG_FIRST(files);
    while ((it = *prev)) {

        if (it->trash_) {

            AUG_REMOVE_PREVPTR(it, prev, files);
            AUG_INSERT_TAIL(&trash, it);

        } else
            prev = &AUG_NEXT(it);
    }

    /* Free trashed items. */

    if (!AUG_EMPTY(&trash)) {

        aug_lock();
        AUG_CONCAT(&free_, &trash);
        aug_unlock();
    }
}

static int
locked_(struct aug_files* files)
{
    return AUG_FIRST(files) == LOCKFILE_;
}

AUGNET_API int
aug_destroyfiles(struct aug_files* files)
{
    struct aug_file_* it;
    AUG_FOREACH(it, files)
        if (!it->trash_ && it->user_)
            aug_decref(it->user_);

    if (!AUG_EMPTY(files)) {

        aug_lock();
        AUG_CONCAT(&free_, files);
        aug_unlock();
    }
    return 0;
}

AUGNET_API int
aug_insertfile(struct aug_files* files, int fd, aug_filecb_t cb,
               aug_object* user)
{
    struct aug_file_* file;

    aug_lock();
    if (!(file = allocate_())) {
        aug_unlock();
        return -1;
    }
    aug_unlock();

    file->trash_ = 0;
    file->fd_ = fd;
    file->cb_ = cb;
    if ((file->user_ = user))
        aug_incref(user);

    AUG_INSERT_TAIL(files, file);
    return 0;
}

AUGNET_API int
aug_removefile(struct aug_files* files, int fd)
{
    /* Locate the matching entry. */

    struct aug_file_* it;
    AUG_FOREACH(it, files)
        if (it->fd_ == fd && !it->trash_)
            break;

    if (!it) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                       AUG_MSG("no file for descriptor '%d'"), (int)fd);
        return -1;
    }

    if (it->user_)
        aug_decref(it->user_);

    if (locked_(files)) {

        /* Items are merely marked for removal while a aug_foreachfile()
           operation is in progress. */

        it->trash_ = 1;

    } else {

        /* Otherwise, move to free list immediately. */

        AUG_REMOVE(files, it, aug_file_);
        aug_lock();
        AUG_INSERT_TAIL(&free_, it);
        aug_unlock();
    }

    return 0;
}

AUGNET_API int
aug_foreachfile(struct aug_files* files)
{
    struct aug_file_* it, * end;
    int locked = locked_(files);

    /* Ensure list is locked so that no entries are actually removed during
       iteration. */

    if (!locked)
        lock_(files);

    end = AUG_LAST(files, aug_file_);
    AUG_FOREACH(it, files) {

        /* Ignore lock marker and trashed entries. */

        if (it == LOCKFILE_ || it->trash_)
            continue;

        if (!(it->cb_(it->user_, it->fd_))) {

            if (it->user_)
                aug_decref(it->user_);
            it->trash_ = 1;
        }

        /* Avoid iterating over items beyond those that were there at the
           start. */

        if (it == end)
            break;
    }

    if (!locked /* On entry. */) {

        unlock_(files);

        /* Rotate files to promote fairness - this must only be done once the
           lock marker has been removed from the front of the list. */

        if ((it = AUG_FIRST(files))) {
            AUG_REMOVE_HEAD(files);
            AUG_INSERT_TAIL(files, it);
        }
    }
    return 0;
}
