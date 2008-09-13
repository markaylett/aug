/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_FILE_HPP
#define AUGUTILPP_FILE_HPP

#include "augctxpp/exception.hpp"

#include "augutil/file.h"

#include "augctx/errno.h"

#include "augext/log.h"

namespace aug {

    template <aug_result (*T)(void*, const char*, const char*)>
    aug_result
    confcb(void* arg, const char* name, const char* value) AUG_NOTHROW
    {
        try {
            return T(arg, name, value);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    template <typename T, aug_result (T::*U)(const char*, const char*)>
    aug_result
    confmemcb(void* arg, const char* name, const char* value) AUG_NOTHROW
    {
        try {
            return (static_cast<T*>(arg)->*U)(name, value);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    template <typename T>
    aug_result
    confmemcb(void* arg, const char* name, const char* value) AUG_NOTHROW
    {
        try {
            return static_cast<T*>(arg)->confcb(name, value);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    inline aug_result
    readconf(const char* path, aug_confcb_t cb, void* arg)
    {
        return verify(aug_readconf(path, cb, arg));
    }

    inline aug_result
    readconf(const char* path, aug_confcb_t cb, const null_&)
    {
        return verify(aug_readconf(path, cb, 0));
    }

    template <typename T>
    aug_result
    readconf(const char* path, T& x)
    {
        return verify(aug_readconf(path, confmemcb<T>, &x));
    }
}

#endif // AUGUTILPP_FILE_HPP
