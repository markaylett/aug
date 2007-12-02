/* -*- c++ -*- */
#ifndef AUG_ADDROB_H
#define AUG_ADDROB_H

#include "augobj.h"

#if defined(__cplusplus)

#include "augobjpp.hpp"

# if !defined(AUG_NOTHROW)
#  define AUG_NOTHROW
# endif /* !AUG_NOTHROW */

/* For pointer conversions, see 4.10/2:

   "An rvalue of type 'pointer to cv T,' where T is an object type, can be
   converted to an rvalue of type 'pointer to cv void.' The result of
   converting a 'pointer to cv T' to a 'pointer to cv void' points to the
   start of the storage location where the object of type T resides, as if the
   object is a most derived object (1.8) of type T (that is, not a base class
   subobject)."

   So the void * will point to the beginning of your class B. And since B is
   not guaranteed to start with the POD, you may not get what you want. */

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

AUG_OBJECTDECL(aug_addrob);
struct aug_addrobvtbl {
    AUG_OBJECT(aug_addrob);
    void* (*get_)(aug_addrob*);
};

#define aug_getaddrob(obj) \
    ((aug_addrob*)obj)->vtbl_->get_(obj)

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_addrob> {
        typedef aug_addrobvtbl vtbl;
        static const char*
        id()
        {
            return aug_addrobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_addrob> addrobref;

    inline void*
    getaddrob(addrobref ref) AUG_NOTHROW
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
        operator addrobref()
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
        objectref
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_addrob>(id))
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
        static aug::smartob<aug_addrob>
        create(const T& impl = T())
        {
            basic_addrob* ptr(new basic_addrob(impl));
            return aug::object_attach<aug_addrob>(ptr->addrob_);
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
        objectref
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_addrob>(id))
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
        operator addrobref()
        {
            return addrob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_ADDROB_H */
