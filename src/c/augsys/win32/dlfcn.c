/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <assert.h>
#include <stdlib.h> /* NULL */

struct aug_dlib_ {
    aug_mpool* mpool_;
    HMODULE handle_;
};

AUGSYS_API aug_result
aug_dlclose(aug_dlib_t dlib)
{
    aug_result result = AUG_SUCCESS;
    aug_mpool* mpool = dlib->mpool_;

    if (!FreeLibrary(dlib->handle_)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        result = AUG_FAILERROR;
    }

    aug_freemem(mpool, dlib);
    aug_release(mpool);

    return result;
}

AUGSYS_API aug_dlib_t
aug_dlopen(aug_mpool* mpool, const char* path)
{
    aug_dlib_t dlib = aug_allocmem(mpool, sizeof(struct aug_dlib_));
    void* handle;

    if (!dlib)
        return NULL;

    if (!(handle = LoadLibrary(path))) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        aug_freemem(mpool, dlib);
        return NULL;
    }

    dlib->mpool_ = mpool;
    dlib->handle_ = handle;

    aug_retain(mpool);
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
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return NULL;
    }

    return local.out_;
}
