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
#include "augctx.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_setposixerror(aug_tlx, __FILE__, 101, EINVAL);

    if (aug_errno(aug_tlerr)) {
        fprintf(stderr, "non-zero errno before init\n");
        return 1;
    }

    aug_autotlx();

    aug_setposixerror(aug_tlx, __FILE__, 101, EINVAL);

    if (0 != strcmp(aug_tlerr->file_, __FILE__)) {
        fprintf(stderr, "unexpected aug_errfile value: %s\n",
                aug_tlerr->file_);
        return 1;
    }

    if (101 != aug_tlerr->line_) {
        fprintf(stderr, "unexpected aug_errline value: %d\n",
                (int)aug_tlerr->line_);
        return 1;
    }

    if (0 != strcmp(aug_tlerr->src_, "posix")) {
        fprintf(stderr, "unexpected aug_errsrc value: %d\n",
                (int)aug_tlerr->src_);
        return 1;
    }

    if (0 != strcmp(aug_tlerr->desc_, strerror(EINVAL))) {
        fprintf(stderr, "unexpected aug_errdesc value: %s\n",
                aug_tlerr->desc_);
        return 1;
    }

    return 0;
}
