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
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"
#include "augext.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_hires_t hires;
    double start, stop;

    if (!aug_autotlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    hires = aug_createhires(mpool);
    aug_release(mpool);

    if (!hires) {
        aug_perrinfo(aug_tlx, "aug_createhires() failed", NULL);
        return 1;
    }

    if (!aug_elapsed(hires, &start)) {
        aug_perrinfo(aug_tlx, "aug_elapsed() failed", NULL);
        return 1;
    }

    /* Sleep for 500 ms. */

    aug_msleep(500);

    if (!aug_elapsed(hires, &stop)) {
        aug_perrinfo(aug_tlx, "aug_elapsed() failed", NULL);
        return 1;
    }

    /* Allow 20ms tollerance. */

    stop -= start;
    if (stop < 0.48 || 0.52 < stop) {
        aug_ctxerror(aug_tlx, "unexpected interval: stop=%0.6f", stop);
        return 1;
    }

    aug_destroyhires(hires);
    return 0;
}
