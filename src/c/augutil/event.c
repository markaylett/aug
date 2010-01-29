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

#define WAITER_ ((void*)1)

struct event_ {
    struct event_* next_;
    int type_;
    aug_object* ob_;
};

static void
destroyevent_(struct event_* event)
{
    aug_mpool* mpool = aug_getcrtmalloc();
    if (event->ob_)
        aug_release(event->ob_);
    aug_freemem(mpool, event);
    aug_release(mpool);
}

static struct event_*
createevent_(int type, aug_object* ob)
{
    /* Must use standard c-malloc to ensure safety from signal handler;
       aug_tlx may not have been initialised on signal handler's thread. */

    aug_mpool* mpool = aug_getcrtmalloc();
    struct event_* event = aug_allocmem(mpool, sizeof(struct event_));
    aug_release(mpool);

    if (event) {
        event->type_ = type;
        if ((event->ob_ = ob))
            aug_retain(ob);
    }
    return event;
}

static void
pushevent_(struct event_** head, struct event_* event)
{
    event->next_ = *head;
    *head = event;
}

static struct event_*
popevent_(struct event_** head)
{
    struct event_* event = *head;
    if (event)
        *head = event->next_;
    return event;
}

static void
reverse_(struct event_** head)
{
    struct event_* revd = NULL;
    struct event_* event;
    while ((event = popevent_(head)))
        pushevent_(&revd, event);
    *head = revd;
}

static aug_rsize
flush_(aug_md md, size_t len)
{
    void* buf = alloca(len);
    aug_rsize rsize;

    while (AUG_ISINTR(rsize = aug_mread(md, buf, len)))
        ;

    return rsize;
}

static aug_rsize
writeone_(aug_md md)
{
    const char ch = 1;
    aug_rsize rsize;

    while (AUG_ISINTR(rsize = aug_mwrite(md, &ch, 1)))
        ;

    return rsize;
}

/* A pipe is used to interrupt the muxer.  Atomicity is ensured by only
   writing a single byte to the pipe for each queued event.  The actual events
   are maintained in a separate list.  The event itself cannot be written to
   the pipe because this would cause problems if ever the pipe were to become
   full.  In this scenario, a partial event could be written with no chance of
   recovery when the muxer is serviced on the same thread. */

struct aug_events_ {
    aug_mpool* mpool_;
    aug_md mds_[2];
    void* head_;
    struct event_* store_;
    unsigned waiters_;
};

static void
pushcasptr_(aug_events_t events, struct event_* event)
{
    struct event_* next;
    do {
        next = aug_acqptr(&events->head_);
        event->next_ = next == WAITER_ ? NULL : next;
    } while (!aug_casptr(&events->head_, next, event));
    if (next == WAITER_)
        writeone_(events->mds_[1]);
}

static struct event_*
loadcasptr_(aug_events_t events)
{
    struct event_* stack;
    for (;;) {
        stack = aug_acqptr(&events->head_);
        if (stack) {
            if (aug_casptr(&events->head_, stack, NULL))
                break;
        } else {
            if (aug_casptr(&events->head_, NULL, WAITER_))
                break;
        }
    }
    return stack;
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
    events->head_ = NULL;
    events->store_ = NULL;
    events->waiters_ = 0;

    aug_retain(mpool);
    return events;
}

AUGUTIL_API void
aug_destroyevents(aug_events_t events)
{
    aug_mpool* mpool = events->mpool_;
    struct event_* event;

    while ((event = popevent_(&events->store_)))
        destroyevent_(event);

    while ((event = popevent_((struct event_**)&events->head_)))
        destroyevent_(event);

    if (AUG_ISFAIL(aug_mclose(events->mds_[0]))
        || AUG_ISFAIL(aug_mclose(events->mds_[1])))
        aug_perrinfo(aug_tlx, "aug_mclose() failed", NULL);

    aug_freemem(mpool, events);
    aug_release(mpool);
}

AUGUTIL_API aug_result
aug_readevent(aug_events_t events, struct aug_event* event)
{
    struct event_* next;

    /* Consume from store if not empty. */

    next = popevent_((struct event_**)&events->head_);
    if (!next) {

        /* Populate store from shared stack. */

        events->store_ = loadcasptr_(events);
        reverse_(&events->store_);
    }

    /* Set output. */

    event->type_ = next->type_;
    if ((event->ob_ = next->ob_))
        aug_retain(event->ob_);

    destroyevent_(next);
    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_writeevent(aug_events_t events, const struct aug_event* event)
{
    struct event_* next = createevent_(event->type_, event->ob_);
    aug_result result;

    if (!next)
        return AUG_FAILERROR;

    /* Non-blocking. */

    result = writeone_(events->mds_[1]);

    return result;
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
