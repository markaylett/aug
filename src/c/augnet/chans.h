/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
 * Destroy @a chans list.
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
 * Remove first matching @a id in @a chans list.
 *
 * @param chans Channel list.
 * @param id Channel to be removed.
 *
 * @return Either @ref 0 or @ref AUG_FAILNONE.
 */

AUGNET_API aug_result
aug_removechan(aug_chans_t chans, aug_id id);

/**
 * Find first matching @a id in @a chans list.
 *
 * @param chans Channel list.
 * @param id Channel to be found.
 *
 * @return Either channel or null if not found.
 */

AUGNET_API aug_chan*
aug_findchan(aug_chans_t chans, aug_id id);

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

AUGNET_API unsigned
aug_getreadychans(aug_chans_t chans);

#endif /* AUGNET_CHANS_H */
