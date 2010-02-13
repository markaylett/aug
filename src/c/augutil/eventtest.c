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
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_events_t events;
    int in, out;

    if (!aug_autotlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    events = aug_createevents(mpool);
    aug_release(mpool);

    if (!events) {
        aug_perrinfo(aug_tlx, "aug_createevents() failed", NULL);
        return 1;
    }

    for (in = 1; in <= 1000; ++in) {

        struct aug_event event;
        aug_result result;

        event.type_ = in;
        event.ob_ = NULL;

        if (AUG_ISFAIL(result = aug_writeevent(events, &event))) {

            if (AUG_ISBLOCK(result))
                break;

            aug_perrinfo(aug_tlx, "aug_writeevent() failed", NULL);
            goto fail;
        }
    }

    for (out = 1;; ++out) {

        struct aug_event event;
        aug_result result = aug_readevent(events, &event);

        if (AUG_ISFAIL(result)) {

            if (AUG_ISBLOCK(result))
                break;

            aug_perrinfo(aug_tlx, "aug_readevent() failed", NULL);
            goto fail;
        }

        if (event.type_ != out) {
            fprintf(stderr, "unexpected event type from aug_readevent()\n");
            goto fail;
        }
    }

    /* Where 16 is some arbitrary choice for the minimum number of events. */

    if (in < 16 || in != out) {
        fprintf(stderr, "event count mismatch\n");
        goto fail;
    }

    aug_destroyevents(events);
    return 0;

 fail:
    aug_destroyevents(events);
    return 1;
}
