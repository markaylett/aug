/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Sat May 17 15:37:38 GMT Daylight Time 2008 */

#ifndef AUGOB_RPCOB_H
#define AUGOB_RPCOB_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */
#include "augob/seqob.h"

/**
 * @defgroup aug_rpcob aug_rpcob
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_rpcob);

struct aug_rpcobvtbl {
    AUG_VTBL(aug_rpcob);
    const char* (*method_)(aug_rpcob*);
    aug_seqob* (*args_)(aug_rpcob*);
};

#define aug_rpcobmethod(this_) \
    (this_)->vtbl_->method_(this_)

#define aug_rpcobargs(this_) \
    (this_)->vtbl_->args_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_rpcob> {
        typedef aug_rpcobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_rpcobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_rpcob> rpcobref;

    inline const char*
    rpcobmethod(aug::obref<aug_rpcob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->method_(this_.get());
    }

    inline aug::smartob<aug_seqob>
    rpcobargs(aug::obref<aug_rpcob> this_) AUG_NOTHROW
    {
        return aug::object_attach<aug_seqob>(this_.get()->vtbl_->args_(this_.get()));
    }

    template <typename T>
    class rpcob {

        aug_rpcob rpcob_;

        rpcob(const rpcob&);

        rpcob&
        operator =(const rpcob&);

        static void*
        cast_(aug_rpcob* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_rpcob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_rpcob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static const char*
        method_(aug_rpcob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->rpcobmethod_();
        }
        static aug_seqob*
        args_(aug_rpcob* this_) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->rpcobargs_());
        }
        static const aug_rpcobvtbl*
        vtbl()
        {
            static const aug_rpcobvtbl local = {
                cast_,
                retain_,
                release_,
                method_,
                args_
            };
            return &local;
        }
    public:
        explicit
        rpcob(T* impl = 0)
        {
            this->rpcob_.vtbl_ = this->vtbl();
            this->rpcob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->rpcob_.impl_ = impl;
        }
        aug_rpcob*
        get() AUG_NOTHROW
        {
            return &this->rpcob_;
        }
        operator aug::obref<aug_rpcob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_rpcob {
        rpcob<basic_rpcob<T> > rpcob_;
        T impl_;
        int refs_;
        explicit
        basic_rpcob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->rpcob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_rpcob>(id))
                return aug::object_retain<aug_object>(this->rpcob_);
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
        const char*
        rpcobmethod_() AUG_NOTHROW
        {
            return this->impl_.rpcobmethod_();
        }
        aug::smartob<aug_seqob>
        rpcobargs_() AUG_NOTHROW
        {
            return this->impl_.rpcobargs_();
        }
        static aug::smartob<aug_rpcob>
        create(const T& impl = T())
        {
            basic_rpcob* ptr(new basic_rpcob(impl));
            return aug::object_attach<aug_rpcob>(ptr->rpcob_);
        }
    };

    template <typename T>
    class scoped_rpcob {
        rpcob<scoped_rpcob<T> > rpcob_;
        T impl_;
    public:
        explicit
        scoped_rpcob(const T& impl = T())
            : impl_(impl)
        {
            this->rpcob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_rpcob>(id))
                return aug::object_retain<aug_object>(this->rpcob_);
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
        const char*
        rpcobmethod_() AUG_NOTHROW
        {
            return this->impl_.rpcobmethod_();
        }
        aug::smartob<aug_seqob>
        rpcobargs_() AUG_NOTHROW
        {
            return this->impl_.rpcobargs_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->rpcob_).get();
        }
        aug_rpcob*
        get() AUG_NOTHROW
        {
            return this->rpcob_.get();
        }
        operator aug::obref<aug_rpcob>() AUG_NOTHROW
        {
            return this->rpcob_;
        }
    };

    typedef aug::smartob<aug_rpcob> rpcobptr;
}
#endif /* __cplusplus */

#endif /* AUGOB_RPCOB_H */
