/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Fri Jun 05 07:04:23 +0100 2009 */

#ifndef AUGEXT_BOXPTR_H
#define AUGEXT_BOXPTR_H

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

AUG_INTERFACE(aug_boxptr);

/* Interface definitions. */

/**
 * @defgroup aug_boxptr aug_boxptr
 *
 * @ingroup Object
 *
 * @{
 */

struct aug_boxptrvtbl {
    AUG_VTBL(aug_boxptr);
    void* (*unboxptr_)(aug_boxptr*);
};

#define aug_unboxptr(this_) \
    (this_)->vtbl_->unboxptr_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_boxptr> {
        typedef aug_boxptrvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_boxptrid;
        }
    };
}

namespace aug {

    template <typename T>
    class boxptr {

        aug_boxptr boxptr_;

        boxptr(const boxptr&);

        boxptr&
        operator =(const boxptr&);

        static void*
        cast_(aug_boxptr* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_boxptr* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_boxptr* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static void*
        unboxptr_(aug_boxptr* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->unboxptr_();
        }
        static const aug_boxptrvtbl*
        vtbl()
        {
            static const aug_boxptrvtbl local = {
                cast_,
                retain_,
                release_,
                unboxptr_
            };
            return &local;
        }
    public:
        explicit
        boxptr(T* impl = 0)
        {
            this->boxptr_.vtbl_ = this->vtbl();
            this->boxptr_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->boxptr_.impl_ = impl;
        }
        aug_boxptr*
        get() AUG_NOTHROW
        {
            return &this->boxptr_;
        }
        operator aug::obref<aug_boxptr>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    typedef aug::obref<aug_boxptr> boxptrref;
    typedef aug::smartob<aug_boxptr> boxptrptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline void*
    unboxptr(aug::obref<aug_boxptr> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->unboxptr_(this_.get());
    }

    template <typename T>
    class boxptr_base {
        boxptr<T> boxptr_;
        int refs_;
    protected:
        ~boxptr_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        boxptr_base()
            : refs_(1)
        {
            this->boxptr_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_boxptr>(id))
                return aug::object_retain<aug_object>(this->boxptr_);
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
        static aug::smartob<aug_boxptr>
        attach(T* ptr)
        {
            return aug::object_attach<aug_boxptr>(ptr->boxptr_);
        }
    };

    template <typename T>
    class scoped_boxptr_base {
        boxptr<T> boxptr_;
    protected:
        ~scoped_boxptr_base()
        {
        }
        scoped_boxptr_base()
        {
            this->boxptr_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_boxptr>(id))
                return aug::object_retain<aug_object>(this->boxptr_);
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
            return aug::obref<aug_object>(this->boxptr_).get();
        }
        aug_boxptr*
        get() AUG_NOTHROW
        {
            return this->boxptr_.get();
        }
        operator aug::obref<aug_boxptr>() AUG_NOTHROW
        {
            return this->boxptr_;
        }
    };

    template <typename T>
    class boxptr_wrapper {
        boxptr<boxptr_wrapper<T> > boxptr_;
        T impl_;
        int refs_;
        explicit
        boxptr_wrapper(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->boxptr_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_boxptr>(id))
                return aug::object_retain<aug_object>(this->boxptr_);
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
        unboxptr_() AUG_NOTHROW
        {
            return this->impl_.unboxptr_();
        }
        static aug::smartob<aug_boxptr>
        create(const T& impl = T())
        {
            boxptr_wrapper* ptr(new boxptr_wrapper(impl));
            return aug::object_attach<aug_boxptr>(ptr->boxptr_);
        }
    };

    template <typename T>
    class scoped_boxptr_wrapper {
        boxptr<scoped_boxptr_wrapper<T> > boxptr_;
        T impl_;
    public:
        explicit
        scoped_boxptr_wrapper(const T& impl = T())
            : impl_(impl)
        {
            this->boxptr_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_boxptr>(id))
                return aug::object_retain<aug_object>(this->boxptr_);
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
        unboxptr_() AUG_NOTHROW
        {
            return this->impl_.unboxptr_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->boxptr_).get();
        }
        aug_boxptr*
        get() AUG_NOTHROW
        {
            return this->boxptr_.get();
        }
        operator aug::obref<aug_boxptr>() AUG_NOTHROW
        {
            return this->boxptr_;
        }
    };

    typedef aug::smartob<aug_boxptr> boxptrptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_BOXPTR_H */
