/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augutil.h"
#include "augctx.h"

#include <stdio.h>

static int refs_ = 0;

static void*
cast_(aug_object* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_object* obj)
{
    ++refs_;
}

static void
release_(aug_object* obj)
{
    --refs_;
}

static const struct aug_objectvtbl vtbl_ = {
    cast_,
    retain_,
    release_
};

static aug_object task1_ = { &vtbl_, NULL };
static aug_object task2_ = { &vtbl_, NULL };
static aug_object task3_ = { &vtbl_, NULL };

static int count_ = 0;
static int last_ = 0;

static aug_bool
cb_(aug_object* task)
{
    ++count_;
    if (task == &task1_) {
        last_ = 1;
    } else if (task == &task2_) {
        last_ = 2;
    } else if (task == &task3_) {
        last_ = 3;
    } else {
        aug_die("invalid task");
    }
    return AUG_TRUE;
}

static void
foreach_(aug_tasks_t tasks)
{
    count_ = 0;
    last_ = 0;
    aug_foreachtask(tasks, cb_);
}

static aug_bool
rm1_(aug_object* task)
{
    return task == &task1_ ? AUG_FALSE : AUG_TRUE;
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_tasks_t tasks;

    aug_check(0 <= aug_start());

    mpool = aug_getmpool(aug_tlx);
    aug_check(mpool);

    tasks = aug_createtasks(mpool);
    aug_check(tasks);

    aug_check(0 <= aug_inserttask(tasks, &task1_));
    aug_check(0 <= aug_inserttask(tasks, &task2_));
    aug_check(0 <= aug_inserttask(tasks, &task3_));
    aug_check(3 == refs_);

    foreach_(tasks);
    aug_check(3 == count_);
    aug_check(3 == last_);

    /* Fairness rotation. */

    foreach_(tasks);
    aug_check(3 == count_);
    aug_check(1 == last_);

    /* Fairness rotation. */

    foreach_(tasks);
    aug_check(3 == count_);
    aug_check(2 == last_);

    /* Remove middle. */

    aug_check(0 <= aug_removetask(tasks, &task2_));

    foreach_(tasks);
    aug_check(2 == count_);
    aug_check(3 == last_);

    /* Remove during loop. */

    aug_foreachtask(tasks, rm1_);

    /* No longer exists. */

    aug_check(AUG_FAILNONE == aug_removetask(tasks, &task1_));

    foreach_(tasks);
    aug_check(1 == count_);
    aug_check(3 == last_);

    aug_destroytasks(tasks);

     /* Objects released. */

    aug_check(0 == refs_);
    aug_release(mpool);
    return 0;
}
