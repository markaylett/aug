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
#include "augctx.h"

#include <errno.h> /* ENOMEM */
#include <stdio.h>

int
main(int argc, char* argv[])
{
    /* Ensure that the initialisation functions can be called multiple
       times. */

    if (!aug_init() || !aug_init() || !aug_init()) {
        fprintf(stderr, "aug_init() failed\n");
        return 1;
    }

    aug_term();
    aug_term();
    aug_term();

    if (!aug_autotlx()) {
        fprintf(stderr, "aug_autotlx() failed\n");
        return 1;
    }

    aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENOMEM);
    aug_perrinfo(aug_tlx, "operation failed", NULL);
    return 0;
}
