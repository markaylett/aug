/* -*- c++ -*- */
#ifndef AUG_BLOB_H
#define AUG_BLOB_H

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

AUG_OBJECTDECL(aug_blob);
struct aug_blobvtbl {
    AUG_OBJECT(aug_blob);
    const void* (*data_)(aug_blob*, size_t*);
    size_t (*size_)(aug_blob*);
};

#define aug_blobdata(obj, size) \
    ((aug_blob*)obj)->vtbl_->data_(obj, size)

#define aug_blobsize(obj) \
    ((aug_blob*)obj)->vtbl_->size_(obj)

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_blob> {
        typedef aug_blobvtbl vtbl;
        static const char*
        id()
        {
            return aug_blobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_blob> blobref;

    inline const void*
    blobdata(blobref ref, size_t* size) AUG_NOTHROW
    {
        aug_blob* obj(ref.get());
        return obj->vtbl_->data_(obj, size);
    }

    inline size_t
    blobsize(blobref ref) AUG_NOTHROW
    {
        aug_blob* obj(ref.get());
        return obj->vtbl_->size_(obj);
    }

    template <typename T>
    class blob {

        aug_blob blob_;

        blob(const blob&);

        blob&
        operator =(const blob&);

        static void*
        cast_(aug_blob* obj, const char* id) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->cast(id).get();
        }
        static int
        incref_(aug_blob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->incref();
        }
        static int
        decref_(aug_blob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->decref();
        }
        static const void*
        data_(aug_blob* obj, size_t* size) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->blobdata(size);
        }
        static size_t
        size_(aug_blob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->blobsize();
        }
        static const aug_blobvtbl*
        vtbl()
        {
            static const aug_blobvtbl local = {
                cast_,
                incref_,
                decref_,
                data_,
                size_
            };
            return &local;
        }
    public:
        explicit
        blob(T* impl = 0)
        {
            blob_.vtbl_ = vtbl();
            blob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            blob_.impl_ = impl;
        }
        aug_blob*
        get()
        {
            return &blob_;
        }
        operator blobref()
        {
            return get();
        }
    };

    template <typename T>
    class basic_blob {
        blob<basic_blob<T> > blob_;
        T impl_;
        unsigned refs_;
        explicit
        basic_blob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            blob_.reset(this);
        }
    public:
        objectref
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_blob>(id))
                return blob_;
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
        const void*
        blobdata(size_t* size) AUG_NOTHROW
        {
            return impl_.blobdata(size);
        }
        size_t
        blobsize() AUG_NOTHROW
        {
            return impl_.blobsize();
        }
        static aug::smartob<aug_blob>
        create(const T& impl = T())
        {
            basic_blob* ptr(new basic_blob(impl));
            return aug::object_attach<aug_blob>(ptr->blob_);
        }
    };

    template <typename T>
    class scoped_blob {
        blob<scoped_blob<T> > blob_;
        T impl_;
    public:
        explicit
        scoped_blob(const T& impl = T())
            : impl_(impl)
        {
            blob_.reset(this);
        }
        objectref
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_blob>(id))
                return blob_;
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
        const void*
        blobdata(size_t* size) AUG_NOTHROW
        {
            return impl_.blobdata(size);
        }
        size_t
        blobsize() AUG_NOTHROW
        {
            return impl_.blobsize();
        }
        aug_blob*
        get()
        {
            return blob_.get();
        }
        operator blobref()
        {
            return blob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_BLOB_H */
