/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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
