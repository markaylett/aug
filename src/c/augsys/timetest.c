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
test(void)
{
    aug_clock* clock;
    long tz;
    time_t in, out;
    struct tm tm;
    time(&in);

    clock = aug_getclock(aug_tlx);
    tz = aug_gettimezone(clock);
    aug_release(clock);

    aug_ctxinfo(aug_tlx, "timezone=[%ld]", tz);

    if (!aug_gmtime(&in, &tm)) {
        aug_perrinfo(aug_tlx, "aug_gmtime() failed", NULL);
        exit(1);
    }

    if (-1 == (out = aug_timegm(&tm))) {
        aug_perrinfo(aug_tlx, "aug_timegm() failed", NULL);
        exit(1);
    }

    if (in != out) {
        fprintf(stderr, "time mismatch: in=[%ld], out=[%ld]\n",
                (long)in, (long)out);
        exit(1);
    }
}

int
main(int argc, char* argv[])
{
    if (!aug_autotlx())
        return 1;
    test();
    return 0;
}
