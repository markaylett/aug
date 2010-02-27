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
    aug_setctxerror(aug_tlx, file, line, "dlfcn", 1, dlerror());
}

AUGSYS_API aug_result
aug_dlclose(aug_dlib_t dlib)
{
    aug_result result = 0;
    aug_mpool* mpool = dlib->mpool_;

    if (0 != dlclose(dlib->handle_)) {
        seterrinfo_(__FILE__, __LINE__);
        result = -1;
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
