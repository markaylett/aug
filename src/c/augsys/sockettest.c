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

#define MSG1_ "first chunk, "
#define MSG2_ "second chunk"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_sd sv[2];
    struct iovec iov[2];
    char buf[AUG_MAXLINE];

    if (!aug_autotlx())
        return 1;

    if (AUG_ISFAIL(aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv))) {
        aug_perrinfo(aug_tlx, "aug_socketpair() failed", NULL);
        return 1;
    }

    iov[0].iov_base = MSG1_;
    iov[0].iov_len = sizeof(MSG1_) - 1;
    iov[1].iov_base = MSG2_;
    iov[1].iov_len = sizeof(MSG2_);

    if (AUG_ISFAIL(aug_swritev(sv[0], iov, 2))) {
        aug_perrinfo(aug_tlx, "aug_writev() failed", NULL);
        return 1;
    }

    if (AUG_ISFAIL(aug_sread(sv[1], buf, iov[0].iov_len + iov[1].iov_len))) {
        aug_perrinfo(aug_tlx, "aug_read() failed", NULL);
        return 1;
    }

    if (0 != strcmp(buf, MSG1_ MSG2_)) {
       fprintf(stderr, "unexpected buffer contents: %s\n", buf);
       return 1;
    }

    aug_sclose(sv[0]);
    aug_sclose(sv[1]);
    return 0;
}
