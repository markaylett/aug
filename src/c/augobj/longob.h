/* -*- c++ -*- */
#ifndef AUG_ADDROB_H
#define AUG_ADDROB_H

#include "augobj.h"

#if defined(__cplusplus)

#include "augobjpp.hpp"

# if !defined(AUG_NOTHROW)
#  define AUG_NOTHROW throw()
# endif /* !AUG_NOTHROW */

// For pointer conversions, see 4.10/2:

// "An rvalue of type 'pointer to cv T,' where T is an object type, can be
// converted to an rvalue of type 'pointer to cv void.' The result of
// converting a 'pointer to cv T' to a 'pointer to cv void' points to the
// start of the storage location where the object of type T resides, as if the
// object is a most derived object (1.8) of type T (that is, not a base class
// subobject)."

// So the void * will point to the beginning of your class B. And since B is
// not guaranteed to start with the POD, you may not get what you want.

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

AUG_OBJECTDECL(aug_longob);
struct aug_longobvtbl {
    AUG_OBJECT(aug_longob);
    long (*get_)(aug_longob*);
};

#define aug_getlongob(obj) \
    ((aug_longob*)obj)->vtbl_->get_(obj)

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_longob> {
        static const char*
        id()
        {
            return aug_longobid;
        }
    };
}

namespace aug {

    inline long
    getlongob(aug::obref<aug_longob> ref) AUG_NOTHROW
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
        operator aug::obref<aug_longob>()
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
        aug::obref<aug_object>
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_longob>(id))
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
        static aug::smartob<aug_longob>
        create(const T& impl = T())
        {
            basic_longob* ptr(new basic_longob(impl));
            return aug::object_attach<aug_longob>(ptr->longob_);
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
        aug::obref<aug_object>
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_longob>(id))
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
        operator aug::obref<aug_longob>()
        {
            return longob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_ADDROB_H */
