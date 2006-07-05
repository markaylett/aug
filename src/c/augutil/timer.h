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

typedef void (*aug_expire_t)(const struct aug_var*, int);

AUGUTIL_API int
aug_freetimers(struct aug_timers* timers);

AUGUTIL_API int
aug_settimer(struct aug_timers* timers, unsigned int ms, aug_expire_t fn,
             const struct aug_var* arg);

AUGUTIL_API int
aug_canceltimer(struct aug_timers* timers, int id);

AUGUTIL_API int
aug_processtimers(struct aug_timers* timers, int force, struct timeval* next);

#endif /* AUGUTIL_TIMER_H */
