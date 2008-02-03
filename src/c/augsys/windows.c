/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/windows.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if defined(_WIN32) && defined(AUGMAR_SHARED)
# include "augsys/debug.h"
AUG_EXTERNC BOOL WINAPI
DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        AUG_INITLEAKDUMP();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        AUG_DUMPLEAKS();
        break;
    };
    return TRUE;
}
#endif /* _WIN32 && AUGMAR_SHARED */
