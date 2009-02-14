/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>
#include <stdlib.h> /* exit() */

static void
test(aug_muxer_t muxer, aug_md md)
{
    struct aug_sticky sticky;
    aug_check(AUG_ISSUCCESS(aug_initsticky(&sticky, muxer, md,
                                           AUG_MDEVENTRDWR)));

    /* Initially, both sticky events are set and no muxer mask. */

    aug_check(AUG_MDEVENTRDWR == aug_getsticky(&sticky));
    aug_check(0 == aug_getmdeventmask(muxer, md));

    /* Alter sticky mask to hide one of the sticky events. */

    aug_check(AUG_ISSUCCESS(aug_setsticky(&sticky, AUG_MDEVENTRD)));

    /* Only read sticky event should now be visible. */

    aug_check(AUG_MDEVENTRD == aug_getsticky(&sticky));
    aug_check(0 == aug_getmdeventmask(muxer, md));

    /* Revert back to initial state. */

    aug_check(AUG_ISSUCCESS(aug_setsticky(&sticky, AUG_MDEVENTRDWR)));

    /* The write sticky event is externally visible again. */

    aug_check(AUG_MDEVENTRDWR == aug_getsticky(&sticky));
    aug_check(0 == aug_getmdeventmask(muxer, md));

    /* Clear one of the sticky events. */

    aug_check(AUG_ISSUCCESS(aug_clearsticky(&sticky, AUG_MDEVENTRD)));

    /* Now, one sticky event and no muxer mask. */

    aug_check(AUG_MDEVENTWR == aug_getsticky(&sticky));
    aug_check(0 == aug_getmdeventmask(muxer, md));

    /* If the mask is reverted, the cleared sticky event should remain
       cleared. */

    aug_check(AUG_ISSUCCESS(aug_setsticky(&sticky, AUG_MDEVENTRDWR)));

    /* One sticky event and no muxer mask. */

    aug_check(AUG_MDEVENTWR == aug_getsticky(&sticky));
    aug_check(0 == aug_getmdeventmask(muxer, md));

    /* Clear the last sticky event, this should set the muxer. */

    aug_check(AUG_ISSUCCESS(aug_clearsticky(&sticky, AUG_MDEVENTWR)));

    aug_check(0 == aug_getsticky(&sticky));
    aug_check(AUG_MDEVENTRDWR == aug_getmdeventmask(muxer, md));

    /* If the mask is set again, the muxer should still remain set. */

    aug_check(AUG_ISSUCCESS(aug_setsticky(&sticky, AUG_MDEVENTWR)));

    aug_check(0 == aug_getsticky(&sticky));
    aug_check(AUG_MDEVENTWR == aug_getmdeventmask(muxer, md));

    /* Wait for writability. */

    aug_check(AUG_ISSUCCESS(aug_waitmdevents(muxer, NULL)));

    aug_check(AUG_MDEVENTWR == aug_getsticky(&sticky));
    aug_check(0 == aug_getmdeventmask(muxer, md));

    aug_termsticky(&sticky);
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_muxer_t muxer;
    aug_sd sv[2];

    if (!aug_autotlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    muxer = aug_createmuxer(mpool);
    aug_release(mpool);

    if (AUG_ISFAIL(aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv))) {
        aug_perrinfo(aug_tlx, "aug_socketpair() failed", NULL);
        aug_destroymuxer(muxer);
        return 1;
    }

    test(muxer, sv[1]);

    aug_sclose(sv[0]);
    aug_sclose(sv[1]);
    aug_destroymuxer(muxer);
    return 0;
}
