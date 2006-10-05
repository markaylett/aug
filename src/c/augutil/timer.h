/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_TIMER_H
#define AUGUTIL_TIMER_H

#include "augutil/list.h"
#include "augutil/var.h"

struct timeval;

struct aug_timer_;
AUG_HEAD(aug_timers, aug_timer_);

typedef void (*aug_timercb_t)(const struct aug_var*, int, unsigned*,
                              struct aug_timers*);

AUGUTIL_API int
aug_freetimers(struct aug_timers* timers);

/**
   If an id of -1 is specified, a new id will be allocated.  Otherwise, if a
   timer with a matching id exists in the list of timers, it will be
   cancelled prior to setting the new timer.
*/

AUGUTIL_API int
aug_settimer(struct aug_timers* timers, int id, unsigned ms,
             aug_timercb_t cb, const struct aug_var* arg);

AUGUTIL_API int
aug_resettimer(struct aug_timers* timers, int id, unsigned ms);

/**
   Returns false if timer did not exist.
*/

AUGUTIL_API int
aug_canceltimer(struct aug_timers* timers, int id);

AUGUTIL_API int
aug_expired(struct aug_timers* timers, int id);

/**
   The force flag forces, at least, the first timer to expire.  This can be
   especially useful when used in combination with a call to
   aug_waitioevents().  Each expired timer will be removed from the list of
   timers prior to calling the callback function.  If the callback function
   returns with a number of milli-seconds greater than zero, the timer will be
   re-scheduled with the same id.
*/

AUGUTIL_API int
aug_processtimers(struct aug_timers* timers, int force, struct timeval* next);

#endif /* AUGUTIL_TIMER_H */
