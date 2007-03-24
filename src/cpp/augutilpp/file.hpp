/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_FILE_HPP
#define AUGUTILPP_FILE_HPP

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/file.h"

#include "augsys/errno.h"
#include "augsys/log.h"

namespace aug {

    template <void (*T)(void*, const char*, const char*)>
    int
    confcb(void* arg, const char* name, const char* value) AUG_NOTHROW
    {
        try {
            T(arg, name, value);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    template <typename T, void (T::*U)(const char*, const char*)>
    int
    confmemcb(void* arg, const char* name, const char* value) AUG_NOTHROW
    {
        try {
            (static_cast<T*>(arg)->*U)(name, value);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    template <typename T>
    int
    confmemcb(void* arg, const char* name, const char* value) AUG_NOTHROW
    {
        try {
            static_cast<T*>(arg)->confcb(name, value);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    inline void
    readconf(const char* path, aug_confcb_t cb, void* arg)
    {
        verify(aug_readconf(path, cb, arg));
    }

    inline void
    readconf(const char* path, aug_confcb_t cb, const null_&)
    {
        verify(aug_readconf(path, cb, 0));
    }

    template <typename T>
    void
    readconf(const char* path, aug_confcb_t cb, T& x)
    {
        verify(aug_readconf(path, confmemcb<T>, &x));
    }
}

#endif // AUGUTILPP_FILE_HPP
