/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/windows.h"

#include <stdlib.h> /* NULL */

struct aug_dlib_ {
    HMODULE handle_;
};

AUGSYS_API int
aug_dlclose(aug_dlib_t dlib)
{
    HMODULE handle = dlib->handle_;
    free(dlib);
    if (!FreeLibrary(handle)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_dlib_t
aug_dlopen(const char* path)
{
    aug_dlib_t dlib = malloc(sizeof(struct aug_dlib_));
    if (!dlib) {
        errno = ENOMEM;
        return NULL;
    }

    if (!(dlib->handle_ = LoadLibrary(path))) {
        free(dlib);
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return NULL;
    }
    return dlib;
}

AUGSYS_API aug_fnptr_t
aug_dlsym(aug_dlib_t dlib, const char* symbol)
{
    /* Avoid warnings: ISO C forbids conversion of function pointer to object
       pointer type. */

    union {
        FARPROC in_;
        aug_fnptr_t out_;
    } local;

    if (!(local.in_ = GetProcAddress(dlib->handle_, symbol))) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return NULL;
    }

    return local.out_;
}
