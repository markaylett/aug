/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_OBJECT_HPP
#define AUGUTILPP_OBJECT_HPP

#include "augutil/object.h"

#include <string>

namespace aug {

    struct simple_boxptr {
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

    struct sblob {
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

    inline aug::smartob<aug_boxint>
    createboxint(int i, void (*destroy)(int))
    {
        return aug::object_attach<aug_boxint>(aug_createboxint(i, destroy));
    }

    template <typename T>
    T
    obtoi(aug::obref<aug_object> ref)
    {
        return static_cast<T>(aug_obtoi(ref.get()));
    }

    template <typename T>
    inline void
    deleter(void* ptr)
    {
        delete static_cast<T*>(ptr);
    }

    inline aug::smartob<aug_boxptr>
    createboxptr(void* p, void (*destroy)(void*))
    {
        return aug::object_attach<aug_boxptr>(aug_createboxptr(p, destroy));
    }

    template <typename T>
    aug::smartob<aug_boxptr>
    createboxptr(std::auto_ptr<T>& x)
    {
        return createboxptr(x.release(), deleter<T>);
    }

    template <typename T>
    T
    obtop(aug::obref<aug_object> ob)
    {
        return static_cast<T>(aug_obtop(ob.get()));
    }

    inline aug::smartob<aug_blob>
    createblob(const void* buf, size_t len)
    {
        return aug::object_attach<aug_blob>(aug_createblob(buf, len));
    }
}

#endif // AUGUTILPP_OBJECT_HPP
