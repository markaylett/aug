/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_CHANS_H
#define AUGNET_CHANS_H

/**
 * @file augnet/chans.h
 *
 * Channel sets.
 */

#include "augnet/config.h"

#include "augext/chan.h"
#include "augext/mpool.h"

typedef struct aug_chans_* aug_chans_t;

/**
 * Create channel list.
 *
 * @param mpool Memory pool.
 *
 * @param handler Channel event handler.
 *
 * @return New channel list.
 */

AUGNET_API aug_chans_t
aug_createchans(aug_mpool* mpool, aug_chandler* handler);

/**
 * Destroy @chans list.
 *
 * @param chans Channel list.
 */

AUGNET_API void
aug_destroychans(aug_chans_t chans);

/**
 * Insert @a ob into @a chans list.
 *
 * @param chans Channel list.
 * @param ob Channel to be inserted.
 *
 * @return See @ref TypesResult.
 */

AUGNET_API aug_result
aug_insertchan(aug_chans_t chans, aug_chan* ob);

/**
 * Remove first matching @a ob from @a chans list.
 *
 * @param chans Channel list.
 * @param id Channel to be removed.
 *
 * @return Either @ref AUG_SUCCESS or @ref AUG_FAILNONE.
 */

AUGNET_API aug_result
aug_removechan(aug_chans_t chans, unsigned id);

/**
 * Process each channel.
 *
 * Other functions associated with @a chans can be safely called during
 * iteration, including recursive calls to aug_processchans().
 *
 * @param chans Channel list.
 */

AUGNET_API void
aug_processchans(aug_chans_t chans);

AUGNET_API void
aug_dumpchans(aug_chans_t chans);

/**
 * Get number of chans.
 *
 * @param chans Channel list.
 *
 * @return Number of chans.
 */

AUGNET_API unsigned
aug_getchans(aug_chans_t chans);

AUGNET_API aug_bool
aug_allblocked(aug_chans_t chans);

#endif /* AUGNET_CHANS_H */
