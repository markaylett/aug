/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errinfo.h"
#include "augsys/errno.h"

#include <stdlib.h> /* NULL */
#include <dlfcn.h>

struct aug_dlib_ {
    void* handle_;
};

static void
seterrinfo_(const char* file, int line)
{
    aug_seterrinfo(NULL, file, line, AUG_SRCDLFCN, 1, dlerror());
}

AUGSYS_API int
aug_dlclose(aug_dlib_t dlib)
{
    void* handle = dlib->handle_;
    free(dlib);
    if (0 != dlclose(handle)) {
        seterrinfo_(__FILE__, __LINE__);
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

    if (!(dlib->handle_ = dlopen(path, RTLD_LAZY))) {
        free(dlib);
        seterrinfo_(__FILE__, __LINE__);
        return NULL;
    }
    return dlib;
}

AUGSYS_API aug_fnptr_t
aug_dlsym(aug_dlib_t dlib, const char* symbol)
{
    /**
       Avoid warnings: ISO C forbids conversion of function pointer to object
       pointer type.
    */

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
