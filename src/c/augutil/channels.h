/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_CHANNELS_H
#define AUGUTIL_CHANNELS_H

/**
 * @file augutil/channels.h
 *
 * Channel sets.
 */

#include "augutil/config.h"

#include "augctx/mpool.h"

#include "augob/channelob.h"

typedef struct aug_channels_* aug_channels_t;

/**
 * Create channel list.
 *
 * @param mpool Memory pool.
 *
 * @return New channel list.
 */

AUGUTIL_API aug_channels_t
aug_createchannels(aug_mpool* mpool);

/**
 * Destroy @channels list.
 *
 * @param channels Channel list.
 */

AUGUTIL_API void
aug_destroychannels(aug_channels_t channels);

/**
 * Insert @a ob into @a channels list.
 *
 * @param channels Channel list.
 * @param ob Channel to be inserted.
 *
 * @return See @ref TypesResult.
 */

AUGUTIL_API aug_result
aug_insertchannel(aug_channels_t channels, aug_channelob* ob);

/**
 * Remove first matching @a ob from @a channels list.
 *
 * @param channels Channel list.
 * @param ob Channel to be removed.
 *
 * @return Either @ref AUG_SUCCESS or @ref AUG_FAILNONE.
 */

AUGUTIL_API aug_result
aug_removechannel(aug_channels_t channels, aug_channelob* ob);

/**
 * Call @a cb function for each channel in list.
 *
 * If the @a cb function returns @ref AUG_FALSE, the item is removed from the
 * list.  Other functions can be called safely during iteration, including
 * recursive calls to aug_foreachchannel().
 *
 * @param channels Channel list.
 * @param cb Callback function.
 */

AUGUTIL_API void
aug_foreachchannel(aug_channels_t channels, aug_channelcb_t cb);

/**
 * Get number of channels.
 *
 * @param channels Channel list.
 *
 * @return Number of channels.
 */

AUGUTIL_API unsigned
aug_getchannels(aug_channels_t channels);

#endif /* AUGUTIL_CHANNELS_H */
