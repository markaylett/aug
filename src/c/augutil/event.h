/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_EVENT_H
#define AUGUTIL_EVENT_H

/**
 * @file augutil/event.h
 *
 * Event type.
 */

#include "augutil/config.h"

#include "augsys/types.h"

#include "augabi.h"

#define AUG_EVENTRECONF 1
#define AUG_EVENTSTATUS 2
#define AUG_EVENTSTOP   3
#define AUG_EVENTSIGNAL 4
#define AUG_EVENTWAKEUP 5

/**
 * Base value for user-defined events.
 */

#define AUG_EVENTUSER   32

struct aug_event {
    int type_;
    aug_object* ob_;
};

/**
 * This function simplifies the mapping of signals to events.  Some signals
 * have cannonical mappings to specific events types.  For example, #SIGHUP
 * maps to the #AUG_EVENTRECONF event type.  Others may get mapped to the more
 * general #AUG_EVENTSIGNAL type.
 */

AUGUTIL_API struct aug_event*
aug_setsigevent(struct aug_event* event, int sig);

AUGUTIL_API struct aug_event*
aug_readevent(aug_md md, struct aug_event* event);

AUGUTIL_API const struct aug_event*
aug_writeevent(aug_md md, const struct aug_event* event);

#endif /* AUGUTIL_EVENT_H */
