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
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

#define TEST_ "some test data"

int
main(int argc, char* argv[])
{
    char buf[64];
    if (!aug_autotlx())
        return 1;

    strcpy(buf, TEST_);
    aug_memfrob(buf, sizeof(buf));
    if (buf[0] != ('s' ^ 42) || 0 == strcmp(buf, TEST_)) {
       fprintf(stderr, "unexpected frob value\n");
       return 1;
    }
    aug_memfrob(buf, sizeof(buf));
    if (0 != strcmp(buf, TEST_)) {
       fprintf(stderr, "unexpected de-frob value\n");
       return 1;
    }
    return 0;
}
