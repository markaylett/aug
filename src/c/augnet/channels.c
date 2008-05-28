/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/channels.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/list.h"

#include "augctx/base.h"
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
    AUG_CTXDEBUG3(aug_tlx, "releasing channel: %u", aug_getid(it->ob_));
    --channels->size_;
    aug_safeassign(it->ob_, NULL);
}

static void
swap_(aug_channelob** lhs, aug_channelob** rhs)
{
    aug_channelob* tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

static void
trash_(aug_channels_t channels)
{
    struct entry_* it, ** prev;

    /* Moved all trashed items to trash list. */

    prev = &AUG_FIRST(&channels->head_);
    while ((it = *prev)) {

        if (!it->ob_) {

            AUG_CTXDEBUG3(aug_tlx, "removing trashed entry");

            AUG_REMOVE_PREVPTR(it, prev, &channels->head_);
            aug_freemem(channels->mpool_, it);

        } else
            prev = &AUG_NEXT(it);
    }
}

AUGNET_API aug_channels_t
aug_createchannels(aug_mpool* mpool)
{
    aug_channels_t channels = aug_allocmem(mpool,
                                           sizeof(struct aug_channels_));
    if (!channels)
        return NULL;

    channels->mpool_ = mpool;
    AUG_INIT(&channels->head_);
    channels->size_ = 0;
    channels->locks_ = 0;

    aug_retain(mpool);
    return channels;
}

AUGNET_API void
aug_destroychannels(aug_channels_t channels)
{
    aug_mpool* mpool = channels->mpool_;
    struct entry_* it;

    while ((it = AUG_FIRST(&channels->head_))) {
        AUG_REMOVE_HEAD(&channels->head_);
        if (it->ob_)
            release_(channels, it);
        aug_freemem(mpool, it);
    }

    aug_freemem(mpool, channels);
    aug_release(mpool);
}

AUGNET_API aug_result
aug_insertchannel(aug_channels_t channels, aug_channelob* ob)
{
    struct entry_* entry = aug_allocmem(channels->mpool_,
                                        sizeof(struct entry_));
    if (!entry)
        return AUG_FAILERROR;

    AUG_CTXDEBUG3(aug_tlx, "retaining channel: %u", aug_getid(ob));

    entry->ob_ = ob;
    aug_retain(ob);

    AUG_INSERT_HEAD(&channels->head_, entry);
    ++channels->size_;
    return AUG_SUCCESS;
}

AUGNET_API aug_result
aug_removechannel(aug_channels_t channels, unsigned id)
{
    /* Locate the matching entry. */

    struct entry_* it;
    AUG_FOREACH(it, &channels->head_) {

        if (!it->ob_) {

            /* Already marked for removal. */

            AUG_CTXDEBUG3(aug_tlx, "ignoring trashed entry");
            continue;
        }

        if (aug_getid(it->ob_) == id)
            goto match;
    }

    return AUG_FAILNONE;

 match:

    if (0 < channels->locks_) {

        /* Items are merely marked for removal while a aug_foreachchannel()
           operation is in progress. */

        AUG_CTXDEBUG3(aug_tlx, "channels locked: delayed removal");

        release_(channels, it);

    } else {

        /* Otherwise, free immediately. */

        AUG_CTXDEBUG3(aug_tlx, "channels unlocked: immediate removal");

        release_(channels, it);
        AUG_REMOVE(&channels->head_, it, entry_);
        aug_freemem(channels->mpool_, it);
    }

    return AUG_SUCCESS;
}

AUGNET_API void
aug_foreachchannel(aug_channels_t channels, aug_channelcb_t cb,
                   aug_object* cbob)
{
    struct entry_* it;

    /* Ensure list is locked so that no entries are actually removed during
       iteration. */

    if (1 == ++channels->locks_) {

        /* Rotate channels to promote fairness.  This is not done for nested
           calls to avoid having the same entries processed twice by outer
           iterations. */

        if ((it = AUG_FIRST(&channels->head_))) {
            AUG_REMOVE_HEAD(&channels->head_);
            AUG_INSERT_TAIL(&channels->head_, it);
        }
    }

    AUG_FOREACH(it, &channels->head_) {

        aug_channelob* ob = it->ob_;
        aug_bool fork = AUG_FALSE;

        /* Ignore trashed entries. */

        if (!ob) {
            AUG_CTXDEBUG3(aug_tlx, "ignoring trashed entry: entry=[%p]", it);
            continue;
        }

        AUG_CTXDEBUG3(aug_tlx, "processing channel: entry=[%p], id=[%u]", it,
                      aug_getid(ob));

        /* Note: the current entry may be marked for removal during this
           call. */

        ob = aug_process(ob, &fork, cb, cbob);

        if (fork) {

            AUG_CTXDEBUG3(aug_tlx, "forking new channel: id=[%u]",
                          aug_getid(ob));

            /* The forked channel is inserted at the head of the list, this
               avoids visitation of the new channel during the current
               iteration. */

            aug_insertchannel(channels, ob);
            aug_release(ob);
            continue; /* Done. */
        }

        /* Not forked.  Assign return of aug_process() to current entry.  The
           returned channel has already been retained. */

        swap_(&it->ob_, &ob);

        /* The local channel reference now contains the channel stored in the
           entry on return from aug_process().  This will be null if the entry
           was marked for removal during the call. */

        if (!ob)
            continue; /* Done. */

        /* A null return from aug_process() indicates that original should be
           released.  Decrement prior to release so that reentrant calls
           reflect the correct size. */

        if (!it->ob_)
            --channels->size_;

        /* Release previous value stored in entry. */

        aug_release(ob);
    }

    if (0 == --channels->locks_) {

        /* Trash when iterations complete. */

        trash_(channels);

    }
}

AUGNET_API void
aug_dumpchannels(aug_channels_t channels)
{
    struct entry_* it;

    aug_ctxinfo(aug_tlx, "dumping channels: size=[%u], locks=[%d]",
                channels->size_, channels->locks_);

    AUG_FOREACH(it, &channels->head_) {

        if (it->ob_)
            aug_ctxinfo(aug_tlx, "active: entry=[%p], id=[%u]", it,
                        aug_getid(it->ob_));
        else
            aug_ctxinfo(aug_tlx, "trash: entry=[%p]", it);
    }
}

AUGNET_API unsigned
aug_getchannels(aug_channels_t channels)
{
    return channels->size_;
}
