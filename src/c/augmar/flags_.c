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
#define AUGMAR_BUILD
#include "augmar/flags_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/types.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"

#include <assert.h>

#define FLAGMASK_ (AUG_RDONLY | AUG_WRONLY | AUG_RDWR \
    | AUG_APPEND | AUG_CREAT | AUG_TRUNC | AUG_EXCL)

AUG_EXTERNC int
aug_toflags_(int* to, int from)
{
#if !defined(_WIN32)
    int flags = 0;
#else /* _WIN32 */
    int flags = O_BINARY;
#endif /* _WIN32 */

    assert(to);
    if (from & ~FLAGMASK_)
        goto fail;

    switch (from & (AUG_RDONLY | AUG_WRONLY | AUG_RDWR)) {
    case AUG_RDONLY:
        flags |= O_RDONLY;
        break;
    case AUG_WRONLY:
    case AUG_RDWR:
        flags |= O_RDWR;
        break;
    default:
        goto fail;
    }
    if (from & AUG_APPEND) {
        assert(AUG_APPEND == (from & AUG_APPEND));
        flags |= O_APPEND;
    }
    if (from & AUG_CREAT) {
        assert(AUG_CREAT == (from & AUG_CREAT));
        flags |= O_CREAT;
    }
    if (from & AUG_TRUNC) {
        assert(AUG_TRUNC == (from & AUG_TRUNC));
        flags |= O_TRUNC;
    }
    if (from & AUG_EXCL) {
        assert(AUG_EXCL == (from & AUG_EXCL));
        flags |= O_EXCL;
    }
    *to = flags;
    return 0;

 fail:
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid open flags"));
    return -1;
}
