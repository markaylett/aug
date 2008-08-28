/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/timer.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/object.h"
#include "augutil/list.h"

#include "augsys/base.h"    /* aug_nextid() */
#include "augsys/time.h"
#include "augsys/utility.h" /* aug_perrinfo() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <assert.h>
#include <stdlib.h>

struct timer_ {
    AUG_ENTRY(timer_);
    int id_;
    unsigned ms_;
    struct timeval tv_;
    aug_timercb_t cb_;
    aug_object* ob_;
};

AUG_HEAD(timers_, timer_);

struct aug_timers_ {
    aug_mpool* mpool_;
    struct timers_ timers_;
};

static struct timer_*
createtimer_(aug_mpool* mpool, aug_object* ob)
{
    struct timer_* timer;
    assert(ob);

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
inserttimer_(struct timers_* timers, struct timer_* timer)
{
    struct timer_* it, * prev;

    it = AUG_FIRST(timers);

    if (!it || timercmp(&timer->tv_, &it->tv_, <)) {

        AUG_INSERT_HEAD(timers, timer);

    } else {

        for (prev = it; (it = AUG_NEXT(prev)); prev = it) {

            if (timercmp(&timer->tv_, &it->tv_, <))
                break;
        }
        AUG_INSERT_AFTER(timers, prev, timer);
    }
}

static aug_result
setexpiry_(struct timeval* tv, unsigned ms)
{
    struct timeval local;
    aug_clock* clock = aug_getclock(aug_tlx);
    aug_result result = aug_gettimeofday(clock, tv);
    aug_release(clock);

    if (result < 0)
        return AUG_FAILERROR;

    aug_tvadd(tv, aug_mstotv(&local, ms));
    return AUG_SUCCESS;
}

AUGUTIL_API aug_timers_t
aug_createtimers(aug_mpool* mpool)
{
    aug_timers_t timers = aug_allocmem(mpool, sizeof(struct aug_timers_));
    if (!timers)
        return NULL;

    timers->mpool_ = mpool;
    AUG_INIT(&timers->timers_);

    aug_retain(mpool);
    return timers;
}

AUGUTIL_API aug_result
aug_destroytimers(aug_timers_t timers)
{
    aug_mpool* mpool = timers->mpool_;
    struct timer_* it;

    /* Destroy in single batch to avoid multiple calls to aug_lock(). */

    while ((it = AUG_FIRST(&timers->timers_))) {
        AUG_REMOVE_HEAD(&timers->timers_);
        destroytimer_(mpool, it);
    }

    aug_freemem(mpool, timers);
    aug_release(mpool);
    return AUG_SUCCESS;
}

AUGUTIL_API int
aug_settimer(aug_timers_t timers, int id, unsigned ms, aug_timercb_t cb,
             aug_object* ob)
{
    struct timeval tv;
    struct timer_* timer;

    if (id <= 0)
        id = aug_nextid();
    else
        aug_canceltimer(timers, id);

    if (setexpiry_(&tv, ms) < 0)
        return AUG_FAILERROR;

    if (!(timer = createtimer_(timers->mpool_, ob)))
        return AUG_FAILERROR;

    timer->id_ = id;
    timer->ms_ = ms;
    timer->tv_.tv_sec = tv.tv_sec;
    timer->tv_.tv_usec = tv.tv_usec;
    timer->cb_ = cb;

    inserttimer_(&timers->timers_, timer);
    return id;
}

AUGUTIL_API aug_result
aug_resettimer(aug_timers_t timers, int id, unsigned ms)
{
    struct timer_* it, ** prev;

    prev = &AUG_FIRST(&timers->timers_);
    while ((it = *prev)) {

        if (it->id_ == id) {

            AUG_REMOVE_PREVPTR(it, prev, &timers->timers_);
            if (ms) /* May be zero. */
                it->ms_ = ms;

            if (setexpiry_(&it->tv_, it->ms_) < 0) {

                destroytimer_(timers->mpool_, it);
                return AUG_FAILERROR;
            }

            inserttimer_(&timers->timers_, it);
            return 0;

        } else
            prev = &AUG_NEXT(it);
    }
    return AUG_FAILNONE;
}

AUGUTIL_API aug_result
aug_canceltimer(aug_timers_t timers, int id)
{
    struct timer_* it, ** prev;

    prev = &AUG_FIRST(&timers->timers_);
    while ((it = *prev)) {

        if (it->id_ == id) {

            AUG_REMOVE_PREVPTR(it, prev, &timers->timers_);
            destroytimer_(timers->mpool_, it);
            return AUG_SUCCESS;

        } else
            prev = &AUG_NEXT(it);
    }
    return AUG_FAILNONE;
}

AUGUTIL_API aug_bool
aug_expired(aug_timers_t timers, int id)
{
    struct timer_* it;
    AUG_FOREACH(it, &timers->timers_)
        if (it->id_ == id)
            return AUG_FALSE;

    return AUG_TRUE;
}

AUGUTIL_API aug_bool
aug_timersempty(aug_timers_t timers)
{
    return AUG_EMPTY(&timers->timers_) ? AUG_TRUE : AUG_FALSE;
}

AUGUTIL_API aug_result
aug_processexpired(aug_timers_t timers, aug_bool force, struct timeval* next)
{
    struct timeval now;
    struct timer_* it;

    if ((it = AUG_FIRST(&timers->timers_))) {

        /* Current time. */

        aug_clock* clock = aug_getclock(aug_tlx);
        aug_result result = aug_gettimeofday(clock, &now);
        aug_release(clock);

        if (result < 0)
            return AUG_FAILERROR;

        /* Force, at least, the first timer to expire. */

        if (force) {
            it->tv_.tv_sec = now.tv_sec;
            it->tv_.tv_usec = now.tv_usec;
        }

        do {

            /* Has this timer expired? */

            if (timercmp(&now, &it->tv_, <))
                break;

            /* Remove expired timer to prevent a new one from being
               recursively added in front. */

            AUG_REMOVE_HEAD(&timers->timers_);

            (*it->cb_)(it->ob_, it->id_, &it->ms_);
            if (it->ms_) {

                /* Update expiry time and insert. */

                if (setexpiry_(&it->tv_, it->ms_) < 0)
                    aug_perrinfo(aug_tlx, "expiry_() failed", NULL);
                else
                    inserttimer_(&timers->timers_, it);

            } else {

                /* A zero ms value cancels the timer. */

                destroytimer_(timers->mpool_, it);
            }

        } while ((it = AUG_FIRST(&timers->timers_)));
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
    return AUG_SUCCESS;
}
