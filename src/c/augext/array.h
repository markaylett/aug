/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Fri Feb 26 07:10:44 +0000 2010 */

#ifndef AUGEXT_ARRAY_H
#define AUGEXT_ARRAY_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/* Includes. */

#include "augtypes.h"

/* Interface declarations. */

AUG_INTERFACE(aug_array);

/* Interface definitions. */

struct aug_var;

/**
 * @defgroup aug_array aug_array
 *
 * @ingroup Object
 *
 * @{
 */

/**
 * Read-only array interface.
 */

struct aug_arrayvtbl {
    AUG_VTBL(aug_array);
    aug_result (*getvalue_)(aug_array*, unsigned, struct aug_var*);
    unsigned (*getsize_)(aug_array*);
};

/**
 * Get value at index @a n.
 * @param this_ The object.
 * @param n Array index.
 * @param value Output value.
 * @return See @ref TypesResult.  #AUG_FAILNONE if there is no matching field.
 */

#define aug_getarrayvalue(this_, n, value) \
    (this_)->vtbl_->getvalue_(this_, n, value)

/**
 * Get number of items.
 * @param this_ The object.
 * @return Number of items.
 */

#define aug_getarraysize(this_) \
    (this_)->vtbl_->getsize_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_array> {
        typedef aug_arrayvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_arrayid;
        }
    };
}

namespace aug {

    template <typename T>
    class array {

        aug_array array_;

        array(const array&);

        array&
        operator =(const array&);

        static void*
        cast_(aug_array* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_array* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_array* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static aug_result
        getvalue_(aug_array* this_, unsigned n, struct aug_var* value) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getarrayvalue_(n, *value);
        }
        static unsigned
        getsize_(aug_array* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getarraysize_();
        }
        static const aug_arrayvtbl*
        vtbl()
        {
            static const aug_arrayvtbl local = {
                cast_,
                retain_,
                release_,
                getvalue_,
                getsize_
            };
            return &local;
        }
    public:
        explicit
        array(T* impl = 0)
        {
            this->array_.vtbl_ = this->vtbl();
            this->array_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->array_.impl_ = impl;
        }
        aug_array*
        get() AUG_NOTHROW
        {
            return &this->array_;
        }
        operator aug::obref<aug_array>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    typedef aug::obref<aug_array> arrayref;
    typedef aug::smartob<aug_array> arrayptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline aug_result
    getarrayvalue(aug::obref<aug_array> this_, unsigned n, struct aug_var& value)
    {
        return verify(this_.get()->vtbl_->getvalue_(this_.get(), n, &value));
    }

    inline unsigned
    getarraysize(aug::obref<aug_array> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getsize_(this_.get());
    }

    template <typename T>
    class array_base {
        array<T> array_;
        int refs_;
    protected:
        ~array_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        array_base()
            : refs_(1)
        {
            this->array_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_array>(id))
                return aug::object_retain<aug_object>(this->array_);
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
        static aug::smartob<aug_array>
        attach(T* ptr)
        {
            return aug::object_attach<aug_array>(ptr->array_);
        }
    };

    template <typename T>
    class scoped_array_base {
        array<T> array_;
    protected:
        ~scoped_array_base()
        {
        }
        scoped_array_base()
        {
            this->array_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_array>(id))
                return aug::object_retain<aug_object>(this->array_);
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
            return aug::obref<aug_object>(this->array_).get();
        }
        aug_array*
        get() AUG_NOTHROW
        {
            return this->array_.get();
        }
        operator aug::obref<aug_array>() AUG_NOTHROW
        {
            return this->array_;
        }
    };

    template <typename T>
    class array_wrapper {
        array<array_wrapper<T> > array_;
        T impl_;
        int refs_;
        explicit
        array_wrapper(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->array_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_array>(id))
                return aug::object_retain<aug_object>(this->array_);
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
        aug_result
        getarrayvalue_(unsigned n, struct aug_var& value) AUG_NOTHROW
        {
            return this->impl_.getarrayvalue_(n, value);
        }
        unsigned
        getarraysize_() AUG_NOTHROW
        {
            return this->impl_.getarraysize_();
        }
        static aug::smartob<aug_array>
        create(const T& impl = T())
        {
            array_wrapper* ptr(new array_wrapper(impl));
            return aug::object_attach<aug_array>(ptr->array_);
        }
    };

    template <typename T>
    class scoped_array_wrapper {
        array<scoped_array_wrapper<T> > array_;
        T impl_;
    public:
        explicit
        scoped_array_wrapper(const T& impl = T())
            : impl_(impl)
        {
            this->array_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_array>(id))
                return aug::object_retain<aug_object>(this->array_);
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
        aug_result
        getarrayvalue_(unsigned n, struct aug_var& value) AUG_NOTHROW
        {
            return this->impl_.getarrayvalue_(n, value);
        }
        unsigned
        getarraysize_() AUG_NOTHROW
        {
            return this->impl_.getarraysize_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->array_).get();
        }
        aug_array*
        get() AUG_NOTHROW
        {
            return this->array_.get();
        }
        operator aug::obref<aug_array>() AUG_NOTHROW
        {
            return this->array_;
        }
    };

    typedef aug::smartob<aug_array> arrayptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_ARRAY_H */
