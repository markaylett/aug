/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx/errinfo.h"

#include <stdlib.h> /* NULL */

#if !defined(__APPLE__) || !defined(__MACH__)
# include <dlfcn.h>
#else /* __APPLE__ && __MACH__ */
# if !HAVE_DLFCN_H
#  include "augsys/osx/dlfcn_.c"
# endif /* !HAVE_DLFCN_H */
#endif /* __APPLE__ && __MACH__ */

struct aug_dlib_ {
    void* handle_;
    aug_ctx* ctx_;
};

static void
seterrinfo_(aug_ctx* ctx, const char* file, int line)
{
    aug_seterrinfo(aug_geterrinfo(ctx), file, line, "dlfcn", 1, dlerror());
}

AUGSYS_API aug_result
aug_dlclose(aug_dlib_t dlib)
{
    aug_result result = AUG_SUCCESS;
    aug_ctx* ctx = dlib->ctx_;

    if (0 != dlclose(dlib->handle_)) {
        seterrinfo_(__FILE__, __LINE__);
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
    void* handle;
    aug_dlib_t dlib;
    assert(ctx);

    if (!(handle = dlopen(path, RTLD_LAZY))) {
        seterrinfo_(__FILE__, __LINE__);
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
        void* in_;
        aug_fnptr_t out_;
    } local;

    if (!(local.in_ = dlsym(dlib->handle_, symbol))) {
        seterrinfo_(__FILE__, __LINE__);
        return NULL;
    }

    return local.out_;
}
