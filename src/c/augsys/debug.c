/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/debug.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/log.h"

#if defined(_MSC_VER) && !defined(NDEBUG)

static void
dumpclient_(void* userdata, size_t size)
{
    char* file;
    int line;
    _ASSERTE(_CrtIsMemoryBlock(userdata, (unsigned)size, NULL, &file, &line));
    aug_warn("%s(%d) : Memory leak detected at 0x%p, %d byes long",
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
