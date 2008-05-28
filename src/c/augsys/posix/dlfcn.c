/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <assert.h>
#include <stdlib.h> /* NULL */

#if !defined(__APPLE__) || !defined(__MACH__)
# include <dlfcn.h>
#else /* __APPLE__ && __MACH__ */
# if !HAVE_DLFCN_H
#  include "augsys/osx/dlfcn_.c"
# endif /* !HAVE_DLFCN_H */
#endif /* __APPLE__ && __MACH__ */

struct aug_dlib_ {
    aug_mpool* mpool_;
    void* handle_;
};

static void
seterrinfo_(const char* file, int line)
{
    aug_seterrinfo(aug_tlerr, file, line, "dlfcn", 1, dlerror());
}

AUGSYS_API aug_result
aug_dlclose(aug_dlib_t dlib)
{
    aug_result result = AUG_SUCCESS;
    aug_mpool* mpool = dlib->mpool_;

    if (0 != dlclose(dlib->handle_)) {
        seterrinfo_(__FILE__, __LINE__);
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

    if (!(handle = dlopen(path, RTLD_LAZY))) {
        seterrinfo_(__FILE__, __LINE__);
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
        void* in_;
        aug_fnptr_t out_;
    } local;

    if (!(local.in_ = dlsym(dlib->handle_, symbol))) {
        seterrinfo_(__FILE__, __LINE__);
        return NULL;
    }

    return local.out_;
}
