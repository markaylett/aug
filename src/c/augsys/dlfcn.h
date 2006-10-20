/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_DLFCN_H
#define AUGSYS_DLFCN_H

#include "augsys/config.h"

typedef void (*aug_fnptr_t)();
typedef struct aug_dlib_* aug_dlib_t;

AUGSYS_API int
aug_dlclose(aug_dlib_t dlib);

AUGSYS_API aug_dlib_t
aug_dlopen(const char* path);

AUGSYS_API aug_fnptr_t
aug_dlsym(aug_dlib_t dlib, const char* symbol);

#endif /* AUGSYS_DLFCN_H */
