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
#include "augutil/timer.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/list.h"
#include "augutil/object.h"

#include "augsys/time.h"
#include "augsys/utility.h" /* aug_nextid() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <assert.h>
#include <stdlib.h>

/* Number of microseconds added when comparing times to compensate for clock
   inaccuracies on the local system.  This value should be sufficient to
   reduce early timer expiry and rapid rescheduling.  So expiry will occur if
   timer is within epsilon of current time. */

#define EPSILONUS_ 5000

struct timer_ {
    AUG_ENTRY(timer_);
    aug_id id_;
    unsigned ms_;
    struct aug_timeval tv_;
    aug_timercb_t cb_;
    aug_object* ob_;
};

AUG_HEAD(list_, timer_);

struct aug_timers_ {
    aug_mpool* mpool_;
    aug_clock* clock_;
    struct list_ list_;
};

static struct timer_*
createtimer_(aug_mpool* mpool, aug_object* ob)
{
    struct timer_* timer;

    if (!(timer = aug_allocmem(mpool, sizeof(struct timer_))))
        return NULL;

    if ((timer->ob_ = ob))
        aug_retain(ob);
    return timer;
}

static void
destroytimer_(aug_mpool* mpool, struct timer_* timer)
{
    if (timer->ob_)
        aug_release(timer->ob_);
    aug_freemem(mpool, timer);
}

static void
inserttimer_(struct list_* list, struct timer_* timer)
{
    struct timer_* it, * prev;

    it = AUG_FIRST(list);

    if (!it || timercmp(&timer->tv_, &it->tv_, <)) {

        AUG_INSERT_HEAD(list, timer);

    } else {

        for (prev = it; (it = AUG_NEXT(prev)); prev = it) {

            if (timercmp(&timer->tv_, &it->tv_, <))
                break;
        }
        AUG_INSERT_AFTER(list, prev, timer);
    }
}

static aug_result
setexpiry_(aug_clock* clock, struct aug_timeval* tv, unsigned ms)
{
    struct aug_timeval local;
    if (aug_gettimeofday(clock, tv) < 0)
        return -1;
    aug_tvadd(tv, aug_mstotv(ms, &local));
    return 0;
}

static aug_bool
expired_(const struct aug_timeval* now, const struct aug_timeval* tv)
{
    struct aug_timeval local = { 0, EPSILONUS_ };
    aug_tvadd(&local, now);
    return timercmp(tv, &local, <) ? AUG_TRUE : AUG_FALSE;
}

AUGUTIL_API aug_timers_t
aug_createtimers(aug_mpool* mpool, aug_clock* clock)
{
    aug_timers_t timers = aug_allocmem(mpool, sizeof(struct aug_timers_));
    if (!timers)
        return NULL;

    timers->mpool_ = mpool;
    timers->clock_ = clock;
    AUG_INIT(&timers->list_);

    aug_retain(mpool);
    aug_retain(clock);
    return timers;
}

AUGUTIL_API void
aug_destroytimers(aug_timers_t timers)
{
    aug_mpool* mpool = timers->mpool_;
    aug_clock* clock = timers->clock_;
    struct timer_* it;

    while ((it = AUG_FIRST(&timers->list_))) {
        AUG_REMOVE_HEAD(&timers->list_);
        destroytimer_(mpool, it);
    }

    aug_freemem(mpool, timers);
    aug_release(clock);
    aug_release(mpool);
}

AUGUTIL_API aug_rint
aug_settimer(aug_timers_t timers, aug_id id, unsigned ms, aug_timercb_t cb,
             aug_object* ob)
{
    struct aug_timeval tv;
    struct timer_* timer;

    if (id <= 0)
        id = aug_nextid();
    else
        aug_canceltimer(timers, id);

    if (setexpiry_(timers->clock_, &tv, ms) < 0
        || !(timer = createtimer_(timers->mpool_, ob)))
        return -1;

    timer->id_ = id;
    timer->ms_ = ms;
    timer->tv_.tv_sec = tv.tv_sec;
    timer->tv_.tv_usec = tv.tv_usec;
    timer->cb_ = cb;

    inserttimer_(&timers->list_, timer);
    return id;
}

AUGUTIL_API aug_result
aug_resettimer(aug_timers_t timers, aug_id id, unsigned ms)
{
    struct timer_* it, ** prev;

    prev = &AUG_FIRST(&timers->list_);
    while ((it = *prev)) {

        if (it->id_ == id) {

            AUG_REMOVE_PREVPTR(it, prev, &timers->list_);
            if (ms) /* May be zero. */
                it->ms_ = ms;

            if (setexpiry_(timers->clock_, &it->tv_, it->ms_) < 0) {

                destroytimer_(timers->mpool_, it);
                return -1;
            }

            inserttimer_(&timers->list_, it);
            return 0;

        } else
            prev = &AUG_NEXT(it);
    }

    aug_setexcept(aug_tlx, AUG_EXNONE);
    return -1;
}

AUGUTIL_API aug_result
aug_canceltimer(aug_timers_t timers, aug_id id)
{
    struct timer_* it, ** prev;

    prev = &AUG_FIRST(&timers->list_);
    while ((it = *prev)) {

        if (it->id_ == id) {

            AUG_REMOVE_PREVPTR(it, prev, &timers->list_);
            destroytimer_(timers->mpool_, it);
            return 0;

        } else
            prev = &AUG_NEXT(it);
    }

    aug_setexcept(aug_tlx, AUG_EXNONE);
    return -1;
}

AUGUTIL_API aug_bool
aug_expired(aug_timers_t timers, aug_id id)
{
    struct timer_* it;
    AUG_FOREACH(it, &timers->list_)
        if (it->id_ == id)
            return AUG_FALSE;

    return AUG_TRUE;
}

AUGUTIL_API aug_bool
aug_timersempty(aug_timers_t timers)
{
    return AUG_EMPTY(&timers->list_);
}

AUGUTIL_API aug_result
aug_processexpired(aug_timers_t timers, aug_bool force,
                   struct aug_timeval* next)
{
    struct aug_timeval now;
    struct timer_* it;

    if ((it = AUG_FIRST(&timers->list_))) {

        /* Current time. */

        if (aug_gettimeofday(timers->clock_, &now) < 0)
            return -1;

        /* Force, at least, the first timer to expire. */

        if (force) {
            it->tv_.tv_sec = now.tv_sec;
            it->tv_.tv_usec = now.tv_usec;
        }

        do {

            /* Has this timer expired? */

            if (!expired_(&now, &it->tv_))
                break;

            /* Remove expired timer to prevent a new one from being
               recursively added in front. */

            AUG_REMOVE_HEAD(&timers->list_);

            (*it->cb_)(it->id_, &it->ms_, it->ob_);
            if (it->ms_) {

                /* Update expiry time and insert. */

                if (setexpiry_(timers->clock_, &it->tv_, it->ms_) < 0)
                    aug_perrinfo(aug_tlx, "expiry_() failed", NULL);
                else
                    inserttimer_(&timers->list_, it);

            } else {

                /* A zero ms value cancels the timer. */

                destroytimer_(timers->mpool_, it);
            }

        } while ((it = AUG_FIRST(&timers->list_)));
    }

    if (next) {

        /* Set optional output argument. */

        if (!it)
            next->tv_sec = next->tv_usec = 0; /* Forever: no timeout. */
        else {

            next->tv_sec = it->tv_.tv_sec;
            next->tv_usec = it->tv_.tv_usec;
            aug_tvsub(next, &now);
        }
    }
    return 0;
}
