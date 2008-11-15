/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Sat Nov 15 10:23:06 GMT Standard Time 2008 */

#ifndef AUGEXT_MPOOL_H
#define AUGEXT_MPOOL_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/* Includes. */


/* Interface declarations. */

AUG_INTERFACE(aug_mpool);

/* Interface definitions. */

/**
 * @defgroup aug_mpool aug_mpool
 *
 * @ingroup Object
 *
 * @{
 */

struct aug_mpoolvtbl {
    AUG_VTBL(aug_mpool);
    void* (*allocmem_)(aug_mpool*, size_t);
    void (*freemem_)(aug_mpool*, void*);
    void* (*reallocmem_)(aug_mpool*, void*, size_t);
    void* (*callocmem_)(aug_mpool*, size_t, size_t);
};

#define aug_allocmem(this_, size) \
    (this_)->vtbl_->allocmem_(this_, size)

#define aug_freemem(this_, ptr) \
    (this_)->vtbl_->freemem_(this_, ptr)

#define aug_reallocmem(this_, ptr, size) \
    (this_)->vtbl_->reallocmem_(this_, ptr, size)

#define aug_callocmem(this_, nmemb, size) \
    (this_)->vtbl_->callocmem_(this_, nmemb, size)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_mpool> {
        typedef aug_mpoolvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_mpoolid;
        }
    };
}

namespace aug {

    template <typename T>
    class mpool {

        aug_mpool mpool_;

        mpool(const mpool&);

        mpool&
        operator =(const mpool&);

        static void*
        cast_(aug_mpool* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_mpool* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_mpool* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static void*
        allocmem_(aug_mpool* this_, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->allocmem_(size);
        }
        static void
        freemem_(aug_mpool* this_, void* ptr) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->freemem_(ptr);
        }
        static void*
        reallocmem_(aug_mpool* this_, void* ptr, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->reallocmem_(ptr, size);
        }
        static void*
        callocmem_(aug_mpool* this_, size_t nmemb, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->callocmem_(nmemb, size);
        }
        static const aug_mpoolvtbl*
        vtbl()
        {
            static const aug_mpoolvtbl local = {
                cast_,
                retain_,
                release_,
                allocmem_,
                freemem_,
                reallocmem_,
                callocmem_
            };
            return &local;
        }
    public:
        explicit
        mpool(T* impl = 0)
        {
            this->mpool_.vtbl_ = this->vtbl();
            this->mpool_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->mpool_.impl_ = impl;
        }
        aug_mpool*
        get() AUG_NOTHROW
        {
            return &this->mpool_;
        }
        operator aug::obref<aug_mpool>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    typedef aug::obref<aug_mpool> mpoolref;
    typedef aug::smartob<aug_mpool> mpoolptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline void*
    allocmem(aug::obref<aug_mpool> this_, size_t size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->allocmem_(this_.get(), size);
    }

    inline void
    freemem(aug::obref<aug_mpool> this_, void* ptr) AUG_NOTHROW
    {
        this_.get()->vtbl_->freemem_(this_.get(), ptr);
    }

    inline void*
    reallocmem(aug::obref<aug_mpool> this_, void* ptr, size_t size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->reallocmem_(this_.get(), ptr, size);
    }

    inline void*
    callocmem(aug::obref<aug_mpool> this_, size_t nmemb, size_t size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->callocmem_(this_.get(), nmemb, size);
    }

    template <typename T>
    class mpool_base {
        mpool<T> mpool_;
        int refs_;
    protected:
        ~mpool_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        mpool_base()
            : refs_(1)
        {
            this->mpool_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_mpool>(id))
                return aug::object_retain<aug_object>(this->mpool_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            ++this->refs_;
        }
        void
        release_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            if (0 == --this->refs_)
                delete static_cast<T*>(this);
        }
        static aug::smartob<aug_mpool>
        attach(T* ptr)
        {
            return aug::object_attach<aug_mpool>(ptr->mpool_);
        }
    };

    template <typename T>
    class scoped_mpool_base {
        mpool<T> mpool_;
    protected:
        ~scoped_mpool_base()
        {
        }
        scoped_mpool_base()
        {
            this->mpool_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_mpool>(id))
                return aug::object_retain<aug_object>(this->mpool_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
        }
        void
        release_() AUG_NOTHROW
        {
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->mpool_).get();
        }
        aug_mpool*
        get() AUG_NOTHROW
        {
            return this->mpool_.get();
        }
        operator aug::obref<aug_mpool>() AUG_NOTHROW
        {
            return this->mpool_;
        }
    };

    template <typename T>
    class mpool_wrapper {
        mpool<mpool_wrapper<T> > mpool_;
        T impl_;
        int refs_;
        explicit
        mpool_wrapper(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->mpool_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_mpool>(id))
                return aug::object_retain<aug_object>(this->mpool_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            ++this->refs_;
        }
        void
        release_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            if (0 == --this->refs_)
                delete this;
        }
        void*
        allocmem_(size_t size) AUG_NOTHROW
        {
            return this->impl_.allocmem_(size);
        }
        void
        freemem_(void* ptr) AUG_NOTHROW
        {
            this->impl_.freemem_(ptr);
        }
        void*
        reallocmem_(void* ptr, size_t size) AUG_NOTHROW
        {
            return this->impl_.reallocmem_(ptr, size);
        }
        void*
        callocmem_(size_t nmemb, size_t size) AUG_NOTHROW
        {
            return this->impl_.callocmem_(nmemb, size);
        }
        static aug::smartob<aug_mpool>
        create(const T& impl = T())
        {
            mpool_wrapper* ptr(new mpool_wrapper(impl));
            return aug::object_attach<aug_mpool>(ptr->mpool_);
        }
    };

    template <typename T>
    class scoped_mpool_wrapper {
        mpool<scoped_mpool_wrapper<T> > mpool_;
        T impl_;
    public:
        explicit
        scoped_mpool_wrapper(const T& impl = T())
            : impl_(impl)
        {
            this->mpool_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_mpool>(id))
                return aug::object_retain<aug_object>(this->mpool_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
        }
        void
        release_() AUG_NOTHROW
        {
        }
        void*
        allocmem_(size_t size) AUG_NOTHROW
        {
            return this->impl_.allocmem_(size);
        }
        void
        freemem_(void* ptr) AUG_NOTHROW
        {
            this->impl_.freemem_(ptr);
        }
        void*
        reallocmem_(void* ptr, size_t size) AUG_NOTHROW
        {
            return this->impl_.reallocmem_(ptr, size);
        }
        void*
        callocmem_(size_t nmemb, size_t size) AUG_NOTHROW
        {
            return this->impl_.callocmem_(nmemb, size);
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->mpool_).get();
        }
        aug_mpool*
        get() AUG_NOTHROW
        {
            return this->mpool_.get();
        }
        operator aug::obref<aug_mpool>() AUG_NOTHROW
        {
            return this->mpool_;
        }
    };

    typedef aug::smartob<aug_mpool> mpoolptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_MPOOL_H */
