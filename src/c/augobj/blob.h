/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Wed Dec 19 02:37:18 GMT Standard Time 2007 */

#ifndef AUG_BLOB_H
#define AUG_BLOB_H

#include "augobj.h"

#if defined(__cplusplus)

#include "augobjpp.hpp"

# if !defined(AUG_NOTHROW)
#  define AUG_NOTHROW
# endif /* !AUG_NOTHROW */

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

#define aug_blobdata(this_, size) \
    ((aug_blob*)this_)->vtbl_->data_(this_, size)

#define aug_blobsize(this_) \
    ((aug_blob*)this_)->vtbl_->size_(this_)

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_blob> {
        typedef aug_blobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_blobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_blob> blobref;

    inline const void*
    blobdata(aug::obref<aug_blob> this_, size_t* size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->data_(this_.get(), size);
    }

    inline size_t
    blobsize(aug::obref<aug_blob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->size_(this_.get());
    }

    template <typename T>
    class blob {

        aug_blob blob_;

        blob(const blob&);

        blob&
        operator =(const blob&);

        static void*
        cast_(aug_blob* this_, const char* id) AUG_NOTHROW
        {
            return aug::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static int
        retain_(aug_blob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->retain_();
        }
        static int
        release_(aug_blob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->release_();
        }
        static const void*
        data_(aug_blob* this_, size_t* size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->blobdata_(size);
        }
        static size_t
        size_(aug_blob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->blobsize_();
        }
        static const aug_blobvtbl*
        vtbl()
        {
            static const aug_blobvtbl local = {
                cast_,
                retain_,
                release_,
                data_,
                size_
            };
            return &local;
        }
    public:
        explicit
        blob(T* impl = 0)
        {
            this->blob_.vtbl_ = this->vtbl();
            this->blob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->blob_.impl_ = impl;
        }
        aug_blob*
        get() AUG_NOTHROW
        {
            return &this->blob_;
        }
        operator aug::obref<aug_blob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_blob {
        blob<basic_blob<T> > blob_;
        T impl_;
        int refs_;
        explicit
        basic_blob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->blob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_blob>(id))
                return aug::object_retain<aug_object>(this->blob_);
            return null;
        }
        int
        retain_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            ++this->refs_;
            return 0;
        }
        int
        release_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            if (0 == --this->refs_)
                delete this;
            return 0;
        }
        const void*
        blobdata_(size_t* size) AUG_NOTHROW
        {
            return this->impl_.blobdata_(size);
        }
        size_t
        blobsize_() AUG_NOTHROW
        {
            return this->impl_.blobsize_();
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
            this->blob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_blob>(id))
                return aug::object_retain<aug_object>(this->blob_);
            return null;
        }
        int
        retain_() AUG_NOTHROW
        {
            return 0;
        }
        int
        release_() AUG_NOTHROW
        {
            return 0;
        }
        const void*
        blobdata_(size_t* size) AUG_NOTHROW
        {
            return this->impl_.blobdata_(size);
        }
        size_t
        blobsize_() AUG_NOTHROW
        {
            return this->impl_.blobsize_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->blob_).get();
        }
        aug_blob*
        get() AUG_NOTHROW
        {
            return this->blob_.get();
        }
        operator aug::obref<aug_blob>() AUG_NOTHROW
        {
            return this->blob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_BLOB_H */
