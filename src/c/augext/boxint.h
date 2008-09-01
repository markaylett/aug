/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Mon Sep 01 06:53:02 GMT Daylight Time 2008 */

#ifndef AUGEXT_BOXINT_H
#define AUGEXT_BOXINT_H

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

AUG_INTERFACE(aug_boxint);

/* Interface definitions. */

/**
 * @defgroup aug_boxint aug_boxint
 *
 * @ingroup Object
 *
 * @{
 */

struct aug_boxintvtbl {
    AUG_VTBL(aug_boxint);
    int (*getint_)(aug_boxint*);
};

#define aug_getboxint(this_) \
    (this_)->vtbl_->getint_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_boxint> {
        typedef aug_boxintvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_boxintid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_boxint> boxintref;

    inline int
    getboxint(aug::obref<aug_boxint> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getint_(this_.get());
    }

    template <typename T>
    class boxint {

        aug_boxint boxint_;

        boxint(const boxint&);

        boxint&
        operator =(const boxint&);

        static void*
        cast_(aug_boxint* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_boxint* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_boxint* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static int
        getint_(aug_boxint* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getboxint_();
        }
        static const aug_boxintvtbl*
        vtbl()
        {
            static const aug_boxintvtbl local = {
                cast_,
                retain_,
                release_,
                getint_
            };
            return &local;
        }
    public:
        explicit
        boxint(T* impl = 0)
        {
            this->boxint_.vtbl_ = this->vtbl();
            this->boxint_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->boxint_.impl_ = impl;
        }
        aug_boxint*
        get() AUG_NOTHROW
        {
            return &this->boxint_;
        }
        operator aug::obref<aug_boxint>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_boxint {
        boxint<basic_boxint<T> > boxint_;
        T impl_;
        int refs_;
        explicit
        basic_boxint(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->boxint_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_boxint>(id))
                return aug::object_retain<aug_object>(this->boxint_);
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
        int
        getboxint_() AUG_NOTHROW
        {
            return this->impl_.getboxint_();
        }
        static aug::smartob<aug_boxint>
        create(const T& impl = T())
        {
            basic_boxint* ptr(new basic_boxint(impl));
            return aug::object_attach<aug_boxint>(ptr->boxint_);
        }
    };

    template <typename T>
    class scoped_boxint {
        boxint<scoped_boxint<T> > boxint_;
        T impl_;
    public:
        explicit
        scoped_boxint(const T& impl = T())
            : impl_(impl)
        {
            this->boxint_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_boxint>(id))
                return aug::object_retain<aug_object>(this->boxint_);
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
        int
        getboxint_() AUG_NOTHROW
        {
            return this->impl_.getboxint_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->boxint_).get();
        }
        aug_boxint*
        get() AUG_NOTHROW
        {
            return this->boxint_.get();
        }
        operator aug::obref<aug_boxint>() AUG_NOTHROW
        {
            return this->boxint_;
        }
    };

    typedef aug::smartob<aug_boxint> boxintptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_BOXINT_H */
