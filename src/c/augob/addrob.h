/* -*- c++ -*- */
/* Automatically generated by aubidl */
/* at Wed Jan 02 20:35:51 GMT Standard Time 2008 */

#ifndef AUG_ADDROB_H
#define AUG_ADDROB_H

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

/**
 * @defgroup aug_addrob aug_addrob
 *
 * @ingroup Object
 *
 * @{
 */

AUB_INTERFACE(aug_addrob);

struct aug_addrobvtbl {
    AUB_VTBL(aug_addrob);
    void* (*get_)(aug_addrob*);
};

#define aug_getaddrob(this_) \
    ((aug_addrob*)this_)->vtbl_->get_(this_)

/** @} */

#if defined(__cplusplus)
namespace aub {
    template <>
    struct object_traits<aug_addrob> {
        typedef aug_addrobvtbl vtbl;
        static const char*
        id() AUB_NOTHROW
        {
            return aug_addrobid;
        }
    };
}

namespace aug {

    typedef aub::obref<aug_addrob> addrobref;

    inline void*
    getaddrob(aub::obref<aug_addrob> this_) AUB_NOTHROW
    {
        return this_.get()->vtbl_->get_(this_.get());
    }

    template <typename T>
    class addrob {

        aug_addrob addrob_;

        addrob(const addrob&);

        addrob&
        operator =(const addrob&);

        static void*
        cast_(aug_addrob* this_, const char* id) AUB_NOTHROW
        {
            return aub::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_addrob* this_) AUB_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_addrob* this_) AUB_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static void*
        get_(aug_addrob* this_) AUB_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getaddrob_();
        }
        static const aug_addrobvtbl*
        vtbl()
        {
            static const aug_addrobvtbl local = {
                cast_,
                retain_,
                release_,
                get_
            };
            return &local;
        }
    public:
        explicit
        addrob(T* impl = 0)
        {
            this->addrob_.vtbl_ = this->vtbl();
            this->addrob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->addrob_.impl_ = impl;
        }
        aug_addrob*
        get() AUB_NOTHROW
        {
            return &this->addrob_;
        }
        operator aub::obref<aug_addrob>() AUB_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_addrob {
        addrob<basic_addrob<T> > addrob_;
        T impl_;
        int refs_;
        explicit
        basic_addrob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->addrob_.reset(this);
        }
    public:
        aub::smartob<aub_object>
        cast_(const char* id) AUB_NOTHROW
        {
            if (aub::equalid<aub_object>(id) || aub::equalid<aug_addrob>(id))
                return aub::object_retain<aub_object>(this->addrob_);
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
        void*
        getaddrob_() AUB_NOTHROW
        {
            return this->impl_.getaddrob_();
        }
        static aub::smartob<aug_addrob>
        create(const T& impl = T())
        {
            basic_addrob* ptr(new basic_addrob(impl));
            return aub::object_attach<aug_addrob>(ptr->addrob_);
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
            this->addrob_.reset(this);
        }
        aub::smartob<aub_object>
        cast_(const char* id) AUB_NOTHROW
        {
            if (aub::equalid<aub_object>(id) || aub::equalid<aug_addrob>(id))
                return aub::object_retain<aub_object>(this->addrob_);
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
        void*
        getaddrob_() AUB_NOTHROW
        {
            return this->impl_.getaddrob_();
        }
        aub_object*
        base() AUB_NOTHROW
        {
            return aub::obref<aub_object>(this->addrob_).get();
        }
        aug_addrob*
        get() AUB_NOTHROW
        {
            return this->addrob_.get();
        }
        operator aub::obref<aug_addrob>() AUB_NOTHROW
        {
            return this->addrob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_ADDROB_H */
