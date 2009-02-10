/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_OBJECT_HPP
#define AUGUTILPP_OBJECT_HPP

#include "augutil/object.h"

#include "augctxpp/mpool.hpp"

#include <memory> // std::auto_ptr<>
#include <string>

namespace aug {

    struct simple_boxptr : mpool_ops {
        void* p_;
        simple_boxptr(void* p)
            : p_(p)
        {
        }
        void*
        getboxptr_() const
        {
            return p_;
        }
    };

    struct sblob : mpool_ops {
        std::string s_;
        sblob(const std::string& s)
            : s_(s)
        {
        }
        const void*
        getblobdata_(size_t* size)
        {
            if (size)
                *size = s_.size();
            return s_.data();
        }
        size_t
        getblobsize_()
        {
            return s_.size();
        }
    };

    inline smartob<aug_boxint>
    createboxint(mpoolref mpool, int i, void (*destroy)(int))
    {
        return object_attach<aug_boxint>
            (aug_createboxint(mpool.get(), i, destroy));
    }

    template <typename T>
    T
    obtoi(obref<aug_object> ref)
    {
        return static_cast<T>(aug_obtoi(ref.get()));
    }

    template <typename T>
    inline void
    deleter(void* ptr)
    {
        delete static_cast<T*>(ptr);
    }

    inline smartob<aug_boxptr>
    createboxptr(mpoolref mpool, void* p, void (*destroy)(void*))
    {
        return object_attach<aug_boxptr>
            (aug_createboxptr(mpool.get(), p, destroy));
    }

    template <typename T>
    smartob<aug_boxptr>
    createboxptr(mpoolref mpool, std::auto_ptr<T>& x)
    {
        return createboxptr(mpool, x.release(), deleter<T>);
    }

    template <typename T>
    T
    obtop(obref<aug_object> ob)
    {
        return static_cast<T>(aug_obtop(ob.get()));
    }

    inline smartob<aug_blob>
    createblob(mpoolref mpool, const void* buf, size_t len)
    {
        return object_attach<aug_blob>(aug_createblob(mpool.get(), buf, len));
    }
}

#endif // AUGUTILPP_OBJECT_HPP
