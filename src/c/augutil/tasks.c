/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/tasks.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/list.h"

#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <stdlib.h>

struct entry_ {
    AUG_ENTRY(entry_);
    aug_object* task_;
};

AUG_HEAD(head_, entry_);

struct aug_tasks_ {
    aug_mpool* mpool_;
    struct head_ head_;
    int locks_;
};

static void
trash_(aug_tasks_t tasks)
{
    struct entry_* it, ** prev;

    /* Moved all trashed items to trash list. */

    prev = &AUG_FIRST(&tasks->head_);
    while ((it = *prev)) {

        if (!it->task_) {

            AUG_REMOVE_PREVPTR(it, prev, &tasks->head_);
            aug_free(tasks->mpool_, it);

        } else
            prev = &AUG_NEXT(it);
    }
}

AUGUTIL_API aug_tasks_t
aug_createtasks(aug_mpool* mpool)
{
    aug_tasks_t tasks = aug_malloc(mpool, sizeof(struct aug_tasks_));
    if (!tasks)
        return NULL;

    tasks->mpool_ = mpool;
    AUG_INIT(&tasks->head_);
    tasks->locks_ = 0;

    aug_retain(mpool);
    return tasks;
}

AUGUTIL_API void
aug_destroytasks(aug_tasks_t tasks)
{
    aug_mpool* mpool = tasks->mpool_;
    struct entry_* it;

    while ((it = AUG_FIRST(&tasks->head_))) {
        AUG_REMOVE_HEAD(&tasks->head_);
        if (it->task_)
            aug_release(it->task_);
        aug_free(mpool, it);
    }

    aug_free(mpool, tasks);
    aug_release(mpool);
}

AUGUTIL_API aug_result
aug_inserttask(aug_tasks_t tasks, aug_object* task)
{
    struct entry_* entry = aug_malloc(tasks->mpool_, sizeof(struct entry_));
    if (!entry)
        return AUG_FAILERROR;

    entry->task_ = task;
    aug_retain(task);

    AUG_INSERT_TAIL(&tasks->head_, entry);
    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_removetask(aug_tasks_t tasks, aug_object* task)
{
    /* Locate the matching entry. */

    struct entry_* it;
    AUG_FOREACH(it, &tasks->head_) {

        if (!it->task_) /* Already marked for removal. */
            goto done;

        if (it->task_ == task)
            break;
    }

    if (!it)
        return AUG_FAILNONE;

    if (0 < tasks->locks_) {

        /* Items are merely marked for removal while a aug_foreachtask()
           operation is in progress. */

        aug_release(it->task_);
        it->task_ = NULL;

    } else {

        /* Otherwise, free immediately. */

        AUG_REMOVE(&tasks->head_, it, entry_);
        aug_release(it->task_);
        aug_free(tasks->mpool_, it);
    }

 done:
    return AUG_SUCCESS;
}

AUGUTIL_API void
aug_foreachtask(aug_tasks_t tasks, aug_taskcb_t cb)
{
    struct entry_* it, * end;

    /* Ensure list is locked so that no entries are actually removed during
       iteration. */

    ++tasks->locks_;

    end = AUG_LAST(&tasks->head_, entry_);
    AUG_FOREACH(it, &tasks->head_) {

        /* Ignore trashed entries. */

        if (!it->task_)
            continue;

        if (!cb(it->task_)) {
            aug_release(it->task_);
            it->task_ = NULL;
        }

        /* Avoid iterating over items beyond those that were there at the
           start. */

        if (it == end)
            break;
    }

    if (0 == --tasks->locks_) {

        /* Trash when iterations complete. */

        trash_(tasks);

        /* Rotate tasks to promote fairness - this must only be done once the
           lock marker has been removed from the front of the list. */

        if ((it = AUG_FIRST(&tasks->head_))) {
            AUG_REMOVE_HEAD(&tasks->head_);
            AUG_INSERT_TAIL(&tasks->head_, it);
        }
    }
}
