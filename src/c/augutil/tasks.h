/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_TASKS_H
#define AUGUTIL_TASKS_H

/**
 * @file augutil/tasks.h
 *
 * Task sets.
 */

#include "augutil/config.h"

#include "augctx/mpool.h"

#include "augtypes.h"

typedef struct aug_tasks_* aug_tasks_t;

typedef aug_bool (*aug_taskcb_t)(aug_object*);

AUGUTIL_API aug_tasks_t
aug_createtasks(aug_mpool* mpool);

AUGUTIL_API void
aug_destroytasks(aug_tasks_t tasks);

AUGUTIL_API aug_result
aug_inserttask(aug_tasks_t tasks, aug_object* task);

AUGUTIL_API aug_result
aug_removetask(aug_tasks_t tasks, aug_object* task);

AUGUTIL_API void
aug_foreachtask(aug_tasks_t tasks, aug_taskcb_t cb);

#endif /* AUGUTIL_TASKS_H */
