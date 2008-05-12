/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/channels.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/list.h"

#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <stdlib.h>

struct entry_ {
    AUG_ENTRY(entry_);
    aug_channelob* ob_;
};

AUG_HEAD(head_, entry_);

struct aug_channels_ {
    aug_mpool* mpool_;
    struct head_ head_;
    unsigned size_;
    int locks_;
};

static void
release_(aug_channels_t channels, struct entry_* it)
{
    --channels->size_;
    aug_saferelease(it->ob_);
}

static void
trash_(aug_channels_t channels)
{
    struct entry_* it, ** prev;

    /* Moved all trashed items to trash list. */

    prev = &AUG_FIRST(&channels->head_);
    while ((it = *prev)) {

        if (!it->ob_) {

            AUG_REMOVE_PREVPTR(it, prev, &channels->head_);
            aug_free(channels->mpool_, it);

        } else
            prev = &AUG_NEXT(it);
    }
}

AUGUTIL_API aug_channels_t
aug_createchannels(aug_mpool* mpool)
{
    aug_channels_t channels = aug_malloc(mpool, sizeof(struct aug_channels_));
    if (!channels)
        return NULL;

    channels->mpool_ = mpool;
    AUG_INIT(&channels->head_);
    channels->size_ = 0;
    channels->locks_ = 0;

    aug_retain(mpool);
    return channels;
}

AUGUTIL_API void
aug_destroychannels(aug_channels_t channels)
{
    aug_mpool* mpool = channels->mpool_;
    struct entry_* it;

    while ((it = AUG_FIRST(&channels->head_))) {
        AUG_REMOVE_HEAD(&channels->head_);
        if (it->ob_)
            release_(channels, it);
        aug_free(mpool, it);
    }

    aug_free(mpool, channels);
    aug_release(mpool);
}

AUGUTIL_API aug_result
aug_insertchannel(aug_channels_t channels, aug_channelob* ob)
{
    struct entry_* entry = aug_malloc(channels->mpool_,
                                      sizeof(struct entry_));
    if (!entry)
        return AUG_FAILERROR;

    entry->ob_ = ob;
    aug_retain(ob);

    AUG_INSERT_TAIL(&channels->head_, entry);
    ++channels->size_;
    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_removechannel(aug_channels_t channels, aug_channelob* ob)
{
    /* Locate the matching entry. */

    struct entry_* it;
    AUG_FOREACH(it, &channels->head_) {

        if (!it->ob_) /* Already marked for removal. */
            continue;

        if (it->ob_ == ob)
            break;
    }

    if (!it)
        return AUG_FAILNONE;

    if (0 < channels->locks_) {

        /* Items are merely marked for removal while a aug_foreachchannel()
           operation is in progress. */

        release_(channels, it);

    } else {

        /* Otherwise, free immediately. */

        release_(channels, it);
        AUG_REMOVE(&channels->head_, it, entry_);
        aug_free(channels->mpool_, it);
    }

    return AUG_SUCCESS;
}

AUGUTIL_API void
aug_foreachchannel(aug_channels_t channels, aug_channelcb_t cb)
{
    struct entry_* it, * end;

    /* Ensure list is locked so that no entries are actually removed during
       iteration. */

    ++channels->locks_;

    end = AUG_LAST(&channels->head_, entry_);
    AUG_FOREACH(it, &channels->head_) {

        aug_channelob* ob = it->ob_;

        /* Ignore trashed entries. */

        if (!ob)
            continue;

        for (;;) {

            aug_bool fork = AUG_FALSE;
            ob = aug_process(ob, cb, &fork);

            /* Null return removes entry. */

            if (!ob) {
                release_(channels, it);
                break;
            }

            /* Returned will have been retained. */

            if (!fork) {

                /* Swap places. */

                aug_channelob* tmp = it->ob_;
                it->ob_ = ob;

                /* Release original. */

                aug_release(tmp);
                break;
            }

            /* Insert child. */

            aug_insertchannel(channels, ob);

            /* Loop to process. */
        }

        /* Avoid iterating over items beyond those that were there at the
           start. */

        if (it == end)
            break;
    }

    if (0 == --channels->locks_) {

        /* Trash when iterations complete. */

        trash_(channels);

        /* Rotate channels to promote fairness - this must only be done once
           the lock marker has been removed from the front of the list. */

        if ((it = AUG_FIRST(&channels->head_))) {
            AUG_REMOVE_HEAD(&channels->head_);
            AUG_INSERT_TAIL(&channels->head_, it);
        }
    }
}

AUGUTIL_API unsigned
aug_getchannels(aug_channels_t channels)
{
    return channels->size_;
}
