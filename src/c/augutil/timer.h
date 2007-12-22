/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_TIMER_H
#define AUGUTIL_TIMER_H

/**
 * @file augutil/timer.h
 *
 * Timers.
 */

#include "augutil/config.h"
#include "augutil/list.h"

#include "augobj.h"

struct timeval;

struct aug_timer_;
AUG_HEAD(aug_timers, aug_timer_);

typedef void (*aug_timercb_t)(aug_object*, int, unsigned*);

AUGUTIL_API int
aug_destroytimers(struct aug_timers* timers);

/**
 * Create new timer.
 *
 * If id <= 0, a new id will be allocated.  Alternatively, a previously
 * allocated timer id can be specified.  In which case, any timer with a
 * matching id will be cancelled prior to setting the new timer.
 *
 * If aug_settimer() succeeds, aug_retain() will be called on @a ob.
 *
 * @param timers TODO
 *
 * @param id TODO
 *
 * @param ms TODO
 *
 * @param cb TODO
 *
 * @param ob TODO
 *
 * @return The timer id.
 */

AUGUTIL_API int
aug_settimer(struct aug_timers* timers, int id, unsigned ms,
             aug_timercb_t cb, aug_object* ob);

/**
 * On failure, the timer will be removed.
 *
 * @param timers The timer list.
 *
 * @param id Timer identifier.
 *
 * @param ms If zero, the previous timeout value will be used.
 *
 * @return #AUG_RETNONE if the timer does not exist.
 */

AUGUTIL_API int
aug_resettimer(struct aug_timers* timers, int id, unsigned ms);

/**
 * Cancel timer.
 *
 * @param timers The timer list.
 *
 * @param id Timer identifier.
 *
 * @return #AUG_RETNONE if the timer does not exist.
 */

AUGUTIL_API int
aug_canceltimer(struct aug_timers* timers, int id);

/**
 * Check whether timer has expired.
 *
 * @param timers The timer list.
 *
 * @param id Timer identifier.
 *
 * @return False on failure.
 */

AUGUTIL_API int
aug_expired(struct aug_timers* timers, int id);

/**
 * Iterate through expired timers.
 *
 * The force flag forces, at least, the first timer to expire.  This can be
 * useful when used in conjunction with a call to aug_waitfdevents().  Each
 * expired timer will be removed from the list of timers prior to calling the
 * callback function.  If the callback function returns with a number of
 * milli-seconds greater than zero, the timer will be re-scheduled with the
 * same id.
 *
 * @param timers The timer list.
 *
 * @param force Boolean indicated whether first timer should be forced to
 * expire.
 *
 * @param next Optional output parameter.  If specified will be populated with
 * next expiry time.
 */

AUGUTIL_API int
aug_foreachexpired(struct aug_timers* timers, int force,
                   struct timeval* next);

#endif /* AUGUTIL_TIMER_H */
