/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_OBJECT_HPP
#define AUGUTILPP_OBJECT_HPP

#include "augobjpp.hpp"

#include "augutil/object.h"

namespace aug {

    template <typename T>
    inline void
    deleter(void* ptr)
    {
        delete static_cast<T*>(ptr);
    }

    template <>
    struct object_traits<aug_addrob> {
        static const char*
        id()
        {
            return aug_addrobid;
        }
    };

    inline void*
    getaddrob(aug_addrob* obj) AUG_NOTHROW
    {
        return obj->vtbl_->get_(obj);
    }

    template <typename T>
    class addrob_base {

        static void*
        cast_(aug_addrob* obj, const char* id) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->cast(id);
        }

        static int
        incref_(aug_addrob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->incref();
        }

        static int
        decref_(aug_addrob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->decref();
        }

        static void*
        get_(aug_addrob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->getaddrob();
        }

    protected:
        ~addrob_base() AUG_NOTHROW
        {
        }
        static const aug_addrobvtbl*
        addrobvtbl()
        {
            static const aug_addrobvtbl local = {
                cast_,
                incref_,
                decref_,
                get_
            };
            return &local;
        }
    };

    class scoped_addrob : addrob_base<scoped_addrob> {
        aug_addrob addrob_;
        void* p_;
        void (*destroy_)(void*);
    public:
        ~scoped_addrob() AUG_NOTHROW
        {
            if (destroy_)
                destroy_(p_);
        }
        scoped_addrob(void* p, void (*destroy)(void*))
            : p_(p),
              destroy_(destroy)
        {
            addrob_.vtbl_ = addrobvtbl();
            addrob_.impl_ = this;
        }
        void*
        cast(const char* id)
        {
            if (equalid<aug_object>(id) || equalid<aug_addrob>(id))
                return &addrob_;
            return NULL;
        }
        int
        incref()
        {
            return 0;
        }
        int
        decref()
        {
            return 0;
        }
        void*
        getaddrob()
        {
            return p_;
        }
        aug_addrob*
        addrob()
        {
            return &addrob_;
        }
        aug_object*
        object()
        {
            return reinterpret_cast<aug_object*>(addrob());
        }
    };

    inline smartobj<aug_addrob>
    createaddrob(void* p, void (*destroy)(void*))
    {
        return object_attach(aug_createaddrob(p, destroy));
    }

    template <typename T>
    T
    obtoaddr(aug_object* obj)
    {
        return static_cast<T>(aug_obtoaddr(obj));
    }

    template <>
    struct object_traits<aug_longob> {
        static const char*
        id()
        {
            return aug_longobid;
        }
    };

    inline long
    getlongob(aug_longob* obj) AUG_NOTHROW
    {
        return obj->vtbl_->get_(obj);
    }

    template <typename T>
    class longob_base {

        static void*
        cast_(aug_longob* obj, const char* id) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->cast(id);
        }

        static int
        incref_(aug_longob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->incref();
        }

        static int
        decref_(aug_longob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->decref();
        }

        static long
        get_(aug_longob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->getlongob();
        }

    protected:
        ~longob_base() AUG_NOTHROW
        {
        }
        static const aug_longobvtbl*
        longobvtbl()
        {
            static const aug_longobvtbl local = {
                cast_,
                incref_,
                decref_,
                get_
            };
            return &local;
        }
    };

    class scoped_longob : longob_base<scoped_longob> {
        aug_longob longob_;
        long l_;
        void (*destroy_)(long);
    public:
        ~scoped_longob() AUG_NOTHROW
        {
            if (destroy_)
                destroy_(l_);
        }
        scoped_longob(long l, void (*destroy)(long))
            : l_(l),
              destroy_(destroy)
        {
            longob_.vtbl_ = longobvtbl();
            longob_.impl_ = this;
        }
        void*
        cast(const char* id)
        {
            if (equalid<aug_object>(id) || equalid<aug_longob>(id))
                return &longob_;
            return NULL;
        }
        int
        incref()
        {
            return 0;
        }
        int
        decref()
        {
            return 0;
        }
        long
        getlongob()
        {
            return l_;
        }
        aug_longob*
        longob()
        {
            return &longob_;
        }
        aug_object*
        object()
        {
            return reinterpret_cast<aug_object*>(longob());
        }
    };

    inline smartobj<aug_longob>
    createlongob(long l, void (*destroy)(long))
    {
        return object_attach(aug_createlongob(l, destroy));
    }

    template <typename T>
    T
    obtolong(aug_object* obj)
    {
        return static_cast<T>(aug_obtoaddr(obj));
    }
}

#endif // AUGUTILPP_OBJECT_HPP
