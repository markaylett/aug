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
    struct object_traits<aug_longob> {
        static const char*
        id()
        {
            return aug_longobid;
        }
    };

    inline long
    getlongob(obref<aug_longob> ref) AUG_NOTHROW
    {
        aug_longob* obj(ref.get());
        return obj->vtbl_->get_(obj);
    }

    template <typename T>
    class longob {

        aug_longob longob_;

        longob(const longob&);

        longob&
        operator =(const longob&);

        static void*
        cast_(aug_longob* obj, const char* id) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->cast(id).get();
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
        static const aug_longobvtbl*
        vtbl()
        {
            static const aug_longobvtbl local = {
                cast_,
                incref_,
                decref_,
                get_
            };
            return &local;
        }
    public:
        explicit
        longob(T* impl = 0)
        {
            longob_.vtbl_ = vtbl();
            longob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            longob_.impl_ = impl;
        }
        aug_longob*
        get()
        {
            return &longob_;
        }
        operator obref<aug_longob>()
        {
            return get();
        }
    };

    template <typename T>
    class basic_longob {
        longob<basic_longob<T> > longob_;
        T impl_;
        unsigned refs_;
        explicit
        basic_longob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            longob_.reset(this);
        }
    public:
        obref<aug_object>
        cast(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_longob>(id))
                return longob_;
            return null;
        }
        int
        incref() AUG_NOTHROW
        {
            ++refs_;
            return 0;
        }
        int
        decref() AUG_NOTHROW
        {
            if (0 == --refs_)
                delete this;
            return 0;
        }
        long
        getlongob() AUG_NOTHROW
        {
            return impl_.getlongob();
        }
        static smartob<aug_longob>
        create(const T& impl = T())
        {
            basic_longob* ptr(new basic_longob(impl));
            return object_attach<aug_longob>(ptr->longob_);
        }
    };

    template <typename T>
    class scoped_longob {
        longob<scoped_longob<T> > longob_;
        T impl_;
    public:
        explicit
        scoped_longob(const T& impl = T())
            : impl_(impl)
        {
            longob_.reset(this);
        }
        obref<aug_object>
        cast(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_longob>(id))
                return longob_;
            return null;
        }
        int
        incref() AUG_NOTHROW
        {
            return 0;
        }
        int
        decref() AUG_NOTHROW
        {
            return 0;
        }
        long
        getlongob() AUG_NOTHROW
        {
            return impl_.getlongob();
        }
        aug_longob*
        get()
        {
            return longob_.get();
        }
        operator obref<aug_longob>()
        {
            return longob_;
        }
    };

    template <>
    struct object_traits<aug_addrob> {
        static const char*
        id()
        {
            return aug_addrobid;
        }
    };

    inline void*
    getaddrob(obref<aug_addrob> ref) AUG_NOTHROW
    {
        aug_addrob* obj(ref.get());
        return obj->vtbl_->get_(obj);
    }

    template <typename T>
    class addrob {

        aug_addrob addrob_;

        addrob(const addrob&);

        addrob&
        operator =(const addrob&);

        static void*
        cast_(aug_addrob* obj, const char* id) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->cast(id).get();
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
        static const aug_addrobvtbl*
        vtbl()
        {
            static const aug_addrobvtbl local = {
                cast_,
                incref_,
                decref_,
                get_
            };
            return &local;
        }
    public:
        explicit
        addrob(T* impl = 0)
        {
            addrob_.vtbl_ = vtbl();
            addrob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            addrob_.impl_ = impl;
        }
        aug_addrob*
        get()
        {
            return &addrob_;
        }
        operator obref<aug_addrob>()
        {
            return get();
        }
    };

    template <typename T>
    class basic_addrob {
        addrob<basic_addrob<T> > addrob_;
        T impl_;
        unsigned refs_;
        explicit
        basic_addrob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            addrob_.reset(this);
        }
    public:
        obref<aug_object>
        cast(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_addrob>(id))
                return addrob_;
            return null;
        }
        int
        incref() AUG_NOTHROW
        {
            ++refs_;
            return 0;
        }
        int
        decref() AUG_NOTHROW
        {
            if (0 == --refs_)
                delete this;
            return 0;
        }
        void*
        getaddrob() AUG_NOTHROW
        {
            return impl_.getaddrob();
        }
        static smartob<aug_addrob>
        create(const T& impl = T())
        {
            basic_addrob* ptr(new basic_addrob(impl));
            return object_attach<aug_addrob>(ptr->addrob_);
        }
    };

    template <typename T>
    class scoped_addrob {
        addrob<scoped_addrob<T> > addrob_;
        T impl_;
    public:
        explicit
        scoped_addrob(const T& impl = T())
            : impl_(impl)
        {
            addrob_.reset(this);
        }
        obref<aug_object>
        cast(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_addrob>(id))
                return addrob_;
            return null;
        }
        int
        incref() AUG_NOTHROW
        {
            return 0;
        }
        int
        decref() AUG_NOTHROW
        {
            return 0;
        }
        void*
        getaddrob() AUG_NOTHROW
        {
            return impl_.getaddrob();
        }
        aug_addrob*
        get()
        {
            return addrob_.get();
        }
        operator obref<aug_addrob>()
        {
            return addrob_;
        }
    };

    inline smartob<aug_longob>
    createlongob(long l, void (*destroy)(long))
    {
        return object_attach(makeref(aug_createlongob(l, destroy)));
    }

    template <typename T>
    T
    obtolong(obref<aug_object> ref)
    {
        return static_cast<T>(aug_obtolong(ref.get()));
    }

    inline smartob<aug_addrob>
    createaddrob(void* p, void (*destroy)(void*))
    {
        return object_attach(makeref(aug_createaddrob(p, destroy)));
    }

    template <typename T>
    T
    obtoaddr(obref<aug_object> ref)
    {
        return static_cast<T>(aug_obtoaddr(ref.get()));
    }
}

#endif // AUGUTILPP_OBJECT_HPP
