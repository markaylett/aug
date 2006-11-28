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

typedef void (*aug_timercb_t)(int, const struct aug_var*, unsigned*,
                              struct aug_timers*);

AUGUTIL_API int
aug_freetimers(struct aug_timers* timers);

/**
   If id <= 0, a new id will be allocated.  Alternatively, a previously
   allocated timer id can be specified.  In which case, any timer with a
   matching id will be cancelled prior to setting the new timer.

   \return the timer id.
*/

AUGUTIL_API int
aug_settimer(struct aug_timers* timers, int id, unsigned ms,
             aug_timercb_t cb, const struct aug_var* arg);

/**
   \param ms may be zero, it which case the previous timeout value will be
   used.

   \return #AUG_RETNONE if the timer does not exist.
*/

AUGUTIL_API int
aug_resettimer(struct aug_timers* timers, int id, unsigned ms);

/**
   \return #AUG_RETNONE if the timer does not exist.
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
