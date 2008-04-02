/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/windows.h"

#include "augctx/errinfo.h"

#include <assert.h>
#include <stdlib.h> /* NULL */

struct aug_dlib_ {
    HMODULE handle_;
    aug_ctx* ctx_;
};

AUGSYS_API aug_result
aug_dlclose(aug_dlib_t dlib)
{
    aug_result result = AUG_SUCCESS;
    aug_ctx* ctx;
    assert(dlib);

    ctx = dlib->ctx_;

    if (!FreeLibrary(dlib->handle_)) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        result = AUG_FAILURE;
    }

    {
        aug_mpool* mpool = aug_getmpool(ctx);
        aug_free(mpool, dlib);
        aug_release(mpool);
    }

    aug_release(ctx);

    return result;
}

AUGSYS_API aug_dlib_t
aug_dlopen(aug_ctx* ctx, const char* path)
{
    HMODULE handle;
    aug_dlib_t dlib;
    assert(ctx);

    if (!(handle = LoadLibrary(path))) {
        aug_setwin32errinfo(aug_geterrinfo(ctx), __FILE__, __LINE__,
                            GetLastError());
        return NULL;
    }

    {
        aug_mpool* mpool = aug_getmpool(ctx);
        dlib = aug_malloc(mpool, sizeof(struct aug_dlib_));
        aug_release(mpool);
    }

    dlib->handle_ = handle;

    aug_retain(ctx);
    dlib->ctx_ = ctx;

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
        aug_setwin32errinfo(aug_geterrinfo(dlib->ctx_), __FILE__, __LINE__,
                            GetLastError());
        return NULL;
    }

    return local.out_;
}
