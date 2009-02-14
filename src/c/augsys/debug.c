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
#define AUGSYS_BUILD
#include "augsys/debug.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"

#include <stdio.h>

#if defined(_MSC_VER) && !defined(NDEBUG)

static void
dumpclient_(void* userdata, size_t size)
{
    char* file;
    int line;
    aug_ctx* ctx;
    _ASSERTE(_CrtIsMemoryBlock(userdata, (unsigned)size, NULL, &file, &line));
    if ((ctx = aug_tlx))
        aug_ctxwarn(ctx,
                    "%s(%d) : Memory leak detected at 0x%p, %d byes long",
                    file, (int)line, userdata, (int)size);
    else
        fprintf(stderr,
                "%s(%d) : Memory leak detected at 0x%p, %d byes long\n",
                file, (int)line, userdata, (int)size);
}

AUGSYS_API void
aug_initleakdump_(void)
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetDumpClient(dumpclient_);
}

AUGSYS_API void
aug_dumpleaks_(void)
{
    _CrtDumpMemoryLeaks();
}

#endif /* _MSC_VER && !NDEBUG */
