/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_OBJECT_HPP
#define AUGUTILPP_OBJECT_HPP

#include "augutil/object.h"

#include <string>

namespace aug {

    struct simple_addrob {
        void* p_;
        simple_addrob(void* p)
            : p_(p)
        {
        }
        void*
        getaddrob_() const
        {
            return p_;
        }
    };

    struct stringob {
        std::string s_;
        stringob(const std::string& s)
            : s_(s)
        {
        }
        const void*
        blobdata_(size_t* size)
        {
            if (size)
                *size = s_.size();
            return s_.data();
        }
        size_t
        blobsize_()
        {
            return s_.size();
        }
    };

    inline smartob<aug_longob>
    createlongob(long l, void (*destroy)(long))
    {
        return object_attach<aug_longob>(aug_createlongob(l, destroy));
    }

    template <typename T>
    T
    obtolong(obref<aug_object> ref)
    {
        return static_cast<T>(aug_obtolong(ref.get()));
    }

    template <typename T>
    inline void
    deleter(void* ptr)
    {
        delete static_cast<T*>(ptr);
    }

    inline smartob<aug_addrob>
    createaddrob(void* p, void (*destroy)(void*))
    {
        return object_attach<aug_addrob>(aug_createaddrob(p, destroy));
    }

    template <typename T>
    smartob<aug_addrob>
    createaddrob(std::auto_ptr<T>& x)
    {
        return createaddrob(x.release(), deleter<T>);
    }

    template <typename T>
    T
    obtoaddr(obref<aug_object> ref)
    {
        return static_cast<T>(aug_obtoaddr(ref.get()));
    }

    inline smartob<aug_blob>
    createblob(const void* buf, size_t len)
    {
        return object_attach<aug_blob>(aug_createblob(buf, len));
    }
}

#endif // AUGUTILPP_OBJECT_HPP
