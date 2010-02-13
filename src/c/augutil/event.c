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
#define AUGUTIL_BUILD
#include "augutil/event.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/object.h"

#include "augsys/muxer.h"

#include "augctx/atomic.h"
#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/mpool.h" /* aug_getcrtmalloc() */

#include <errno.h>
#include <signal.h>

#if !defined(_WIN32)
# if HAVE_ALLOCA_H
#  include <alloca.h>
# endif /* HAVE_ALLOCA_H */
#else /* _WIN32 */
# include <malloc.h>       /* alloca() */
# define SIGHUP  1
# define SIGUSR1 10
#endif /* _WIN32 */

#define WAKEUP_ ((void*)1)

struct link_ {
    struct link_* next_;
    int type_;
    aug_object* ob_;
};

static void
destroylink_(struct link_* link)
{
    aug_mpool* mpool = aug_getcrtmalloc();
    if (link->ob_)
        aug_release(link->ob_);
    aug_freemem(mpool, link);
    aug_release(mpool);
}

static struct link_*
createlink_(int type, aug_object* ob)
{
    /* Must use standard c-malloc to ensure safety from signal handler;
       aug_tlx may not have been initialised on signal handler's thread. */

    aug_mpool* mpool = aug_getcrtmalloc();
    struct link_* link = aug_allocmem(mpool, sizeof(struct link_));
    aug_release(mpool);

    if (link) {
        link->type_ = type;
        if ((link->ob_ = ob))
            aug_retain(ob);
    }
    return link;
}

static void
pushlink_(struct link_** head, struct link_* link)
{
    link->next_ = *head;
    *head = link;
}

static struct link_*
poplink_(struct link_** head)
{
    struct link_* link = *head;
    if (link)
        *head = link->next_;
    return link;
}

static void
reverse_(struct link_** head)
{
    struct link_* revd = NULL;
    struct link_* link;
    while ((link = poplink_(head)))
        pushlink_(&revd, link);
    *head = revd;
}

static aug_rsize
flush_(aug_md md, unsigned wakeups)
{
    void* buf = alloca(wakeups);
    aug_rsize rsize;

    while (AUG_ISINTR(rsize = aug_mread(md, buf, (size_t)wakeups)))
        ;

    /* Zero or more. */
    return AUG_ISBLOCK(rsize) ? AUG_ZERO : rsize;
}

static aug_result
wakeup_(aug_md md)
{
    const char ch = 1;
    aug_rsize rsize;

    /* aug_mwrite() can be called from threads where no context has been
       initialised, the only caveat being that the errinfo structure will not
       be populated on error. */

    while (AUG_ISINTR(rsize = aug_mwrite(md, &ch, 1)))
        ;

    if (AUG_ISFAIL(rsize) || 1 != AUG_RESULT(rsize)) {

        /* If called from a thread where the context has not been initialised,
           the this call simply has no effect. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EIO,
                       AUG_MSG("failed to wakeup reader"));
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

struct aug_events_ {
    aug_mpool* mpool_;
    aug_md mds_[2];
    void* shared_;
    struct link_* local_;
    unsigned wakeups_;
};

static aug_result
pushcasptr_(aug_events_t events, struct link_* link)
{
    struct link_* next;
    do {
        next = aug_acqptr(&events->shared_);
        /* Ignore wakeup marker when setting link's next pointer. */
        link->next_ = next == WAKEUP_ ? NULL : next;
        /* Until there is no concurrent modification. */
    } while (!aug_casptr(&events->shared_, next, link));

    /* Done if not asked to wakeup. */
    if (next != WAKEUP_)
        return AUG_SUCCESS;

    /* Wakeup marker was set, so wakeup consumer. */
    return wakeup_(events->mds_[1]);
}

static aug_result
loadcasptr_(aug_events_t events, struct link_** head)
{
    for (;;) {
        *head = aug_acqptr(&events->shared_);
        if (head) {
            /* Defensive: return null if wakeup marker has already been
            published by this (the only) consumer. This should only happen if
            consumer has read more than once without polling the event
            descriptor for readability -- and no event was available on each
            occasion. */
            if (WAKEUP_ == *head) {
                *head = NULL;
                break;
            }
            /* Replace head with null. */
            if (aug_casptr(&events->shared_, *head, NULL)) {
                /* Shread stack has now been transferred to head. */
                if (0 < events->wakeups_) {
                    /* Flush each wakeup written by producers. */
                    aug_rsize rsize = flush_(events->mds_[0],
                                             events->wakeups_);
                    if (AUG_ISFAIL(rsize))
                        return AUG_FAILERROR;
                    /* Reduce wakeups by actual number read. The result may be
                       zero as the publisher is not gauranteed to have written
                       by this time.
                    */
                    events->wakeups_ -= AUG_RESULT(rsize);
                }
                break;
            }
        } else {
            /* Replace null head with wakeup marker, indicating that the
               consumer will wait until woken. */
            if (aug_casptr(&events->shared_, NULL, WAKEUP_)) {
                /* Consumer should now proceed to wait on event descriptor
                   readability. */
                ++events->wakeups_;
                break;
            }
        }
        /* Modification detected, so continue. */
    }
    /* Head is either shared stack or null. */
    return AUG_SUCCESS;
}

AUGUTIL_API aug_events_t
aug_createevents(aug_mpool* mpool)
{
    aug_events_t events = aug_allocmem(mpool, sizeof(struct aug_events_));
    if (!events)
        return NULL;

    events->mpool_ = mpool;
    if (AUG_ISFAIL(aug_muxerpipe(events->mds_))) {
        aug_freemem(mpool, events);
        return NULL;
    }
    /* Always wakeup on first event. */
    events->shared_ = WAKEUP_;
    events->local_ = NULL;
    events->wakeups_ = 0;

    aug_retain(mpool);
    return events;
}

AUGUTIL_API void
aug_destroyevents(aug_events_t events)
{
    aug_mpool* mpool = events->mpool_;
    struct link_* link;

    while ((link = poplink_(&events->local_)))
        destroylink_(link);

    /* Head can be accessed like a regular stack during destruction, because
       it is assumed that aug_writeevent() will not be called. */

    while ((link = poplink_((struct link_**)&events->shared_)))
        destroylink_(link);

    if (AUG_ISFAIL(aug_mclose(events->mds_[0]))
        || AUG_ISFAIL(aug_mclose(events->mds_[1])))
        aug_perrinfo(aug_tlx, "aug_mclose() failed", NULL);

    aug_freemem(mpool, events);
    aug_release(mpool);
}

AUGUTIL_API aug_result
aug_readevent(aug_events_t events, struct aug_event* event)
{
    struct link_* next;

    /* Consume from local stack if not empty. */

    next = poplink_(&events->local_);
    if (!next) {

        /* Otherwise populate local from shared stack. */

        aug_verify(loadcasptr_(events, &events->local_));
        if (!events->local_) {
            /* Wakeup marker has already been written. */
            return AUG_FAILBLOCK;
        }

        /* Reverse the stack so that it effectively becomes a queue. */

        reverse_(&events->local_);
        next = poplink_(&events->local_);
    }

    /* Set output event. */

    event->type_ = next->type_;
    if ((event->ob_ = next->ob_))
        aug_retain(event->ob_);

    destroylink_(next);
    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_writeevent(aug_events_t events, const struct aug_event* event)
{
    struct link_* next = createlink_(event->type_, event->ob_);
    return next ? pushcasptr_(events, next) : AUG_FAILERROR;
}

AUGUTIL_API aug_md
aug_eventsmd(aug_events_t events)
{
    return events->mds_[0];
}

AUGUTIL_API struct aug_event*
aug_sigtoevent(int sig, struct aug_event* event)
{
    /* Must use standard c-malloc to ensure safety from signal handler;
       aug_tlx may not have been initialised on signal handler's thread. */

    aug_mpool* mpool = aug_getcrtmalloc();
    switch (sig) {
    case SIGHUP:
        event->type_ = AUG_EVENTRECONF;
        break;
    case SIGUSR1:
        event->type_ = AUG_EVENTSTATUS;
        break;
    case SIGINT:
    case SIGTERM:
        event->type_ = AUG_EVENTSTOP;
        break;
    default:
        event->type_ = AUG_EVENTSIGNAL;
    }
    event->ob_ = (aug_object*)aug_createboxint(mpool, sig, NULL);
    aug_release(mpool);
    return event;
}
