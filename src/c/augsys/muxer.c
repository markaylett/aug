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
#define AUGSYS_BUILD
#include "augsys/muxer.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

static const char LABELS_[][4] = {
    "___",
    "R__",
    "_W_",
    "RW_",
    "__X",
    "R_X",
    "_WX",
    "RWX"
};

static const struct timeval NOWAIT_ = { 0, 0 };

#if !defined(_WIN32)
# include "augsys/posix/muxer.c"
#else /* _WIN32 */
# include "augsys/win32/muxer.c"
#endif /* _WIN32 */

AUGSYS_API aug_rint
aug_pollmdevents(aug_muxer_t muxer)
{
    return aug_waitmdevents(muxer, &NOWAIT_);
}

AUGSYS_API const char*
aug_eventlabel(unsigned short events)
{
    if (sizeof(LABELS_) / sizeof(LABELS_[0]) <= (size_t)events)
        events = 0;

    return LABELS_[events];
}
