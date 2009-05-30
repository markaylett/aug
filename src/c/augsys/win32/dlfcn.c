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
    aug_mpool* mpool = dlib->mpool_;
    aug_result result = AUG_SUCCESS;

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
