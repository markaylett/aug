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

#include "augmar.h"
#include "augctx.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    struct aug_field field;
    aug_mar* mar;

    if (!aug_autotlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    mar = aug_createmar(mpool);
    aug_release(mpool);

    field.name_ = "name";
    field.value_ = "Mark";
    field.size_ = 4;
    if (AUG_ISFAIL(aug_putfield(mar, &field)))
        aug_die("aug_putfield() failed");

    field.name_ = "age";
    field.value_ = "33";
    field.size_ = 2;
    if (AUG_ISFAIL(aug_putfield(mar, &field)))
        aug_die("aug_putfield() failed");

    aug_release(mar);
    return 0;
}
