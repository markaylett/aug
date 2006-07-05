/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/timer.h"

static const char rcsid[] = "$Id:$";

#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/lock.h"
#include "augsys/time.h"

#include <limits.h>
#include <stdlib.h>

struct aug_timer_ {
    AUG_ENTRY(aug_timer_);
    int id_;
    struct timeval tv_;
    aug_expire_t fn_;
    struct aug_var arg_;
};

static struct aug_timers free_ = AUG_HEAD_INITIALIZER(free_);
AUG_ALLOCATOR(allocate_, &free_, aug_timer_, 64)

static int
expiry_(struct timeval* tv, unsigned int ms)
{
    struct timeval local;
    if (-1 == aug_gettimeofday(tv, NULL))
        return -1;

    aug_tvadd(tv, aug_mstotv(&local, ms));
    return 0;
}

static void
insert_(struct aug_timers* timers, struct aug_timer_* timer)
{
    struct aug_timer_* it, * prev;

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

static int
nextid_(void)
{
    static int id_ = 0;

    if (id_ == INT_MAX) {
        id_ = 0;
        return INT_MAX;
    }

    return id_++;
}

AUGUTIL_API int
aug_freetimers(struct aug_timers* timers)
{
    if (!AUG_EMPTY(timers)) {

        aug_lock();
        AUG_CONCAT(&free_, timers);
        aug_unlock();
    }
    return 0;
}

AUGUTIL_API int
aug_settimer(struct aug_timers* timers, unsigned int ms, aug_expire_t fn,
             const struct aug_var* arg)
{
    struct timeval tv;
    struct aug_timer_* timer;

    if (-1 == expiry_(&tv, ms))
        return -1;

    aug_lock();
    if (!(timer = allocate_())) {
        aug_unlock();
        return -1;
    }

    timer->id_ = nextid_();
    aug_unlock();

    timer->tv_.tv_sec = tv.tv_sec;
    timer->tv_.tv_usec = tv.tv_usec;
    timer->fn_ = fn;
    aug_setvar(&timer->arg_, arg);
    insert_(timers, timer);

    return timer->id_;
}

AUGUTIL_API int
aug_canceltimer(struct aug_timers* timers, int id)
{
    struct aug_timer_* it, ** prev;

    prev = &AUG_FIRST(timers);
    while ((it = *prev)) {

        if (it->id_ == id) {

            AUG_REMOVE_PREVPTR(it, prev, timers);

            aug_lock();
            AUG_INSERT_TAIL(&free_, it);
            aug_unlock();
            return 0;

        } else
            prev = &AUG_NEXT(it);
    }

    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("no timer with descriptor '%d'"), id);
    return -1;
}

AUGUTIL_API int
aug_processtimers(struct aug_timers* timers, int force, struct timeval* next)
{
    struct timeval now;
    struct aug_timer_* it;

    if ((it = AUG_FIRST(timers))) {

        if (-1 == aug_gettimeofday(&now, NULL))
            return -1;

        /* Force, at least, the first timer to expire. */

        if (force)
            it->tv_ = now;

        do {

            if (timercmp(&now, &it->tv_, <))
                break;

            (*it->fn_)(&it->arg_, it->id_);

            AUG_REMOVE_HEAD(timers);

            aug_lock();
            AUG_INSERT_TAIL(&free_, it);
            aug_unlock();

        } while ((it = AUG_FIRST(timers)));
    }

    if (next) {

        if (!it)
            next->tv_sec = next->tv_usec = 0; /* Forever. */
        else {

            next->tv_sec = it->tv_.tv_sec;
            next->tv_usec = it->tv_.tv_usec;
            aug_tvsub(next, &now);
        }
    }
    return 0;
}
