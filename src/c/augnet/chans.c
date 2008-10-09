/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/chans.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/list.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <assert.h>
#include <stdlib.h>

struct entry_ {
    AUG_ENTRY(entry_);
    aug_chan* ob_;
};

AUG_HEAD(head_, entry_);

struct aug_chans_ {
    aug_mpool* mpool_;
    aug_chandler* handler_;
    struct head_ head_;
    unsigned size_;
    int locks_;
};

static void
release_(aug_chans_t chans, struct entry_* it)
{
    aug_chan* ob = it->ob_;
    unsigned id = aug_getchanid(ob);

    AUG_CTXDEBUG3(aug_tlx, "releasing channel: %u", id);
    --chans->size_;
    it->ob_ = NULL;

    /* Notify after entry has been removed. */

    aug_clearchan(chans->handler_, id);

    /* Release after callback to avoid dangling references during callback. */

    aug_release(ob);
}

static void
swap_(aug_chan** lhs, aug_chan** rhs)
{
    aug_chan* tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

static void
trash_(aug_chans_t chans)
{
    struct entry_* it, ** prev;

    /* Moved all trashed items to trash list. */

    prev = &AUG_FIRST(&chans->head_);
    while ((it = *prev)) {

        if (!it->ob_) {

            AUG_CTXDEBUG3(aug_tlx, "removing trashed entry");

            AUG_REMOVE_PREVPTR(it, prev, &chans->head_);
            aug_freemem(chans->mpool_, it);

        } else
            prev = &AUG_NEXT(it);
    }
}

AUGNET_API aug_chans_t
aug_createchans(aug_mpool* mpool, aug_chandler* handler)
{
    aug_chans_t chans = aug_allocmem(mpool, sizeof(struct aug_chans_));
    if (!chans)
        return NULL;

    chans->mpool_ = mpool;
    chans->handler_ = handler;
    AUG_INIT(&chans->head_);
    chans->size_ = 0;
    chans->locks_ = 0;

    aug_retain(mpool);
    aug_retain(handler);
    return chans;
}

AUGNET_API void
aug_destroychans(aug_chans_t chans)
{
    aug_mpool* mpool = chans->mpool_;
    aug_chandler* handler = chans->handler_;
    struct entry_* it;

    assert(0 == chans->locks_);

    while ((it = AUG_FIRST(&chans->head_))) {
        AUG_REMOVE_HEAD(&chans->head_);
        if (it->ob_)
            release_(chans, it);
        aug_freemem(mpool, it);
    }

    aug_freemem(mpool, chans);
    aug_release(handler);
    aug_release(mpool);
}

AUGNET_API aug_result
aug_insertchan(aug_chans_t chans, aug_chan* ob)
{
    struct entry_* entry = aug_allocmem(chans->mpool_, sizeof(struct entry_));
    if (!entry)
        return AUG_FAILERROR;

    AUG_CTXDEBUG3(aug_tlx, "retaining channel: %u", aug_getchanid(ob));

    entry->ob_ = ob;
    aug_retain(ob);

    AUG_INSERT_HEAD(&chans->head_, entry);
    ++chans->size_;
    return AUG_SUCCESS;
}

AUGNET_API aug_result
aug_removechan(aug_chans_t chans, unsigned id)
{
    /* Locate the matching entry. */

    struct entry_* it;
    AUG_FOREACH(it, &chans->head_) {

        if (!it->ob_) {

            /* Already marked for removal. */

            AUG_CTXDEBUG3(aug_tlx, "ignoring trashed entry");
            continue;
        }

        if (aug_getchanid(it->ob_) == id)
            goto match;
    }

    return AUG_FAILNONE;

 match:

    if (0 < chans->locks_) {

        /* Items are merely marked for removal while a aug_foreachchannel()
           operation is in progress. */

        AUG_CTXDEBUG3(aug_tlx, "channels locked: delayed removal");

        release_(chans, it);

    } else {

        /* Otherwise, free immediately. */

        AUG_CTXDEBUG3(aug_tlx, "channels unlocked: immediate removal");

        release_(chans, it);
        AUG_REMOVE(&chans->head_, it, entry_);
        aug_freemem(chans->mpool_, it);
    }

    return AUG_SUCCESS;
}

AUGNET_API void
aug_processchans(aug_chans_t chans)
{
    struct entry_* it;

    /* Ensure list is locked so that no entries are actually removed during
       iteration. */

    if (1 == ++chans->locks_) {

        /* Rotate channels to promote fairness.  This is not done for nested
           calls to avoid having the same entries processed twice by outer
           iterations. */

        if ((it = AUG_FIRST(&chans->head_))) {
            AUG_REMOVE_HEAD(&chans->head_);
            AUG_INSERT_TAIL(&chans->head_, it);
        }
    }

    AUG_FOREACH(it, &chans->head_) {

        aug_chan* ob = it->ob_;
        aug_bool fork = AUG_FALSE;

        /* Ignore trashed entries. */

        if (!ob) {
            AUG_CTXDEBUG3(aug_tlx, "ignoring trashed entry: entry=[%p]", it);
            continue;
        }

        AUG_CTXDEBUG3(aug_tlx, "processing channel: entry=[%p], id=[%u]", it,
                      aug_getchanid(ob));

        /* Note: the current entry may be marked for removal during this
           call. */

        ob = aug_processchan(ob, chans->handler_, &fork);

        if (fork) {

            AUG_CTXDEBUG3(aug_tlx, "forking new channel: id=[%u]",
                          aug_getchanid(ob));

            /* The forked channel is inserted at the head of the list, this
               avoids visitation of the new channel during the current
               iteration. */

            aug_insertchan(chans, ob);
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

        if (!it->ob_) {

            unsigned id = aug_getchanid(ob);

            AUG_CTXDEBUG3(aug_tlx, "releasing channel: %u", id);
            --chans->size_;

            /* Notify after entry has been removed. */

            aug_clearchan(chans->handler_, id);
        }

        /* Release previous value stored in entry. */

        aug_release(ob);
    }

    if (0 == --chans->locks_) {

        /* Trash when iterations complete. */

        trash_(chans);
    }
}

AUGNET_API void
aug_dumpchans(aug_chans_t chans)
{
    struct entry_* it;

    aug_ctxinfo(aug_tlx, "dumping channels: size=[%u], locks=[%d]",
                chans->size_, chans->locks_);

    AUG_FOREACH(it, &chans->head_) {

        if (it->ob_)
            aug_ctxinfo(aug_tlx, "active: entry=[%p], id=[%u]", it,
                        aug_getchanid(it->ob_));
        else
            aug_ctxinfo(aug_tlx, "trash: entry=[%p]", it);
    }
}

AUGNET_API unsigned
aug_getchans(aug_chans_t chans)
{
    return chans->size_;
}

AUGNET_API unsigned
aug_getreadychans(aug_chans_t chans)
{
    unsigned ready = 0;
    struct entry_* it;
    AUG_FOREACH(it, &chans->head_) {

        if (it->ob_ && !aug_ischanready(it->ob_))
            ++ready;
    }
    return ready;
}
