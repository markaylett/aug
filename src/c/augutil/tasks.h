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

#include "augsys/file.h"

typedef struct aug_tasks_* aug_tasks_t;

/**
 * Create task list.
 *
 * @param mpool Memory pool.
 *
 * @return New task list.
 */

AUGUTIL_API aug_tasks_t
aug_createtasks(aug_mpool* mpool);

/**
 * Destroy @tasks list.
 *
 * @param tasks Task list.
 */

AUGUTIL_API void
aug_destroytasks(aug_tasks_t tasks);

/**
 * Insert @a task into @a tasks list.
 *
 * @param tasks Task list.
 * @param task Task to be inserted.
 *
 * @return See @ref TypesResult.
 */

AUGUTIL_API aug_result
aug_inserttask(aug_tasks_t tasks, aug_file* task);

/**
 * Remove first matching @a task from @a tasks list.
 *
 * @param tasks Task list.
 * @param task Task to be removed.
 *
 * @return Either @ref AUG_SUCCESS or @ref AUG_FAILNONE.
 */

AUGUTIL_API aug_result
aug_removetask(aug_tasks_t tasks, aug_file* task);

/**
 * Get number of tasks.
 *
 * @param tasks Task list.
 *
 * @return Number of tasks.
 */

AUGUTIL_API unsigned
aug_gettasks(aug_tasks_t tasks);

/**
 * Call @a cb function for each task in list.
 *
 * If the @a cb function returns @ref AUG_FALSE, the item is removed from the
 * list.  Other functions can be called safely during iteration, including
 * recursive calls to aug_foreachtask().
 *
 * @param tasks Task list.
 * @param cb Callback function.
 */

AUGUTIL_API void
aug_foreachtask(aug_tasks_t tasks, aug_filecb_t cb);

#endif /* AUGUTIL_TASKS_H */
