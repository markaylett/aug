/* -*- c++ -*- */
/* Automatically generated by aubidl */
/* at Wed Jan 02 20:35:52 GMT Standard Time 2008 */

#ifndef AUG_ITEROB_H
#define AUG_ITEROB_H

#include "aub.h"

#if defined(__cplusplus)

#include "aubpp.hpp"

# if !defined(AUB_NOTHROW)
#  define AUB_NOTHROW
# endif /* !AUB_NOTHROW */

namespace aub {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */
struct aug_var;

/**
 * @defgroup aug_iterob aug_iterob
 *
 * @ingroup Object
 *
 * @{
 */

AUB_INTERFACE(aug_iterob);

struct aug_iterobvtbl {
    AUB_VTBL(aug_iterob);
    int (*end_)(aug_iterob*);
    void (*next_)(aug_iterob*);
    void (*key_)(aug_iterob*, struct aug_var*);
    void (*value_)(aug_iterob*, struct aug_var*);
};

#define aug_iterobend(this_) \
    ((aug_iterob*)this_)->vtbl_->end_(this_)

#define aug_nextiterob(this_) \
    ((aug_iterob*)this_)->vtbl_->next_(this_)

#define aug_iterobkey(this_, var) \
    ((aug_iterob*)this_)->vtbl_->key_(this_, var)

#define aug_iterobvalue(this_, var) \
    ((aug_iterob*)this_)->vtbl_->value_(this_, var)

/** @} */

#if defined(__cplusplus)
namespace aub {
    template <>
    struct object_traits<aug_iterob> {
        typedef aug_iterobvtbl vtbl;
        static const char*
        id() AUB_NOTHROW
        {
            return aug_iterobid;
        }
    };
}

namespace aug {

    typedef aub::obref<aug_iterob> iterobref;

    inline int
    iterobend(aub::obref<aug_iterob> this_) AUB_NOTHROW
    {
        return this_.get()->vtbl_->end_(this_.get());
    }

    inline void
    nextiterob(aub::obref<aug_iterob> this_) AUB_NOTHROW
    {
        this_.get()->vtbl_->next_(this_.get());
    }

    inline void
    iterobkey(aub::obref<aug_iterob> this_, struct aug_var* var) AUB_NOTHROW
    {
        this_.get()->vtbl_->key_(this_.get(), var);
    }

    inline void
    iterobvalue(aub::obref<aug_iterob> this_, struct aug_var* var) AUB_NOTHROW
    {
        this_.get()->vtbl_->value_(this_.get(), var);
    }

    template <typename T>
    class iterob {

        aug_iterob iterob_;

        iterob(const iterob&);

        iterob&
        operator =(const iterob&);

        static void*
        cast_(aug_iterob* this_, const char* id) AUB_NOTHROW
        {
            return aub::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_iterob* this_) AUB_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_iterob* this_) AUB_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static int
        end_(aug_iterob* this_) AUB_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->iterobend_();
        }
        static void
        next_(aug_iterob* this_) AUB_NOTHROW
        {
            static_cast<T*>(this_->impl_)->nextiterob_();
        }
        static void
        key_(aug_iterob* this_, struct aug_var* var) AUB_NOTHROW
        {
            static_cast<T*>(this_->impl_)->iterobkey_(var);
        }
        static void
        value_(aug_iterob* this_, struct aug_var* var) AUB_NOTHROW
        {
            static_cast<T*>(this_->impl_)->iterobvalue_(var);
        }
        static const aug_iterobvtbl*
        vtbl()
        {
            static const aug_iterobvtbl local = {
                cast_,
                retain_,
                release_,
                end_,
                next_,
                key_,
                value_
            };
            return &local;
        }
    public:
        explicit
        iterob(T* impl = 0)
        {
            this->iterob_.vtbl_ = this->vtbl();
            this->iterob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->iterob_.impl_ = impl;
        }
        aug_iterob*
        get() AUB_NOTHROW
        {
            return &this->iterob_;
        }
        operator aub::obref<aug_iterob>() AUB_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_iterob {
        iterob<basic_iterob<T> > iterob_;
        T impl_;
        int refs_;
        explicit
        basic_iterob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->iterob_.reset(this);
        }
    public:
        aub::smartob<aub_object>
        cast_(const char* id) AUB_NOTHROW
        {
            if (aub::equalid<aub_object>(id) || aub::equalid<aug_iterob>(id))
                return aub::object_retain<aub_object>(this->iterob_);
            return null;
        }
        void
        retain_() AUB_NOTHROW
        {
            assert(0 < this->refs_);
            ++this->refs_;
        }
        void
        release_() AUB_NOTHROW
        {
            assert(0 < this->refs_);
            if (0 == --this->refs_)
                delete this;
        }
        int
        iterobend_() AUB_NOTHROW
        {
            return this->impl_.iterobend_();
        }
        void
        nextiterob_() AUB_NOTHROW
        {
            this->impl_.nextiterob_();
        }
        void
        iterobkey_(struct aug_var* var) AUB_NOTHROW
        {
            this->impl_.iterobkey_(var);
        }
        void
        iterobvalue_(struct aug_var* var) AUB_NOTHROW
        {
            this->impl_.iterobvalue_(var);
        }
        static aub::smartob<aug_iterob>
        create(const T& impl = T())
        {
            basic_iterob* ptr(new basic_iterob(impl));
            return aub::object_attach<aug_iterob>(ptr->iterob_);
        }
    };

    template <typename T>
    class scoped_iterob {
        iterob<scoped_iterob<T> > iterob_;
        T impl_;
    public:
        explicit
        scoped_iterob(const T& impl = T())
            : impl_(impl)
        {
            this->iterob_.reset(this);
        }
        aub::smartob<aub_object>
        cast_(const char* id) AUB_NOTHROW
        {
            if (aub::equalid<aub_object>(id) || aub::equalid<aug_iterob>(id))
                return aub::object_retain<aub_object>(this->iterob_);
            return null;
        }
        void
        retain_() AUB_NOTHROW
        {
        }
        void
        release_() AUB_NOTHROW
        {
        }
        int
        iterobend_() AUB_NOTHROW
        {
            return this->impl_.iterobend_();
        }
        void
        nextiterob_() AUB_NOTHROW
        {
            this->impl_.nextiterob_();
        }
        void
        iterobkey_(struct aug_var* var) AUB_NOTHROW
        {
            this->impl_.iterobkey_(var);
        }
        void
        iterobvalue_(struct aug_var* var) AUB_NOTHROW
        {
            this->impl_.iterobvalue_(var);
        }
        aub_object*
        base() AUB_NOTHROW
        {
            return aub::obref<aub_object>(this->iterob_).get();
        }
        aug_iterob*
        get() AUB_NOTHROW
        {
            return this->iterob_.get();
        }
        operator aub::obref<aug_iterob>() AUB_NOTHROW
        {
            return this->iterob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_ITEROB_H */
