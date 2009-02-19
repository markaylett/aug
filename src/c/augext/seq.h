/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Thu Feb 19 10:33:53 +0000 2009 */

#ifndef AUGEXT_SEQ_H
#define AUGEXT_SEQ_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/* Includes. */

#include "augext/iter.h"

/* Interface declarations. */

AUG_INTERFACE(aug_seq);

/* Interface definitions. */

/**
 * @defgroup aug_seq aug_seq
 *
 * @ingroup Object
 *
 * @{
 */

/**
 * Read-only, base interface for all container types.  Suitable for both
 * sequential and associative containers.
 */

struct aug_seqvtbl {
    AUG_VTBL(aug_seq);
    aug_result (*getvalue_)(aug_seq*, struct aug_var*, const struct aug_var*);
    aug_iter* (*getfirst_)(aug_seq*);
    int (*isempty_)(aug_seq*);
    unsigned (*getsize_)(aug_seq*);
};

/**
 * Get item specified by @a key.
 * @param this_ The object.
 * @param value Output value.
 * @param key Value's key.
 * @return See @ref TypesResult.
 */

#define aug_getseqvalue(this_, value, key) \
    (this_)->vtbl_->getvalue_(this_, value, key)

/**
 * Get first item.
 * @param this_ The object.
 * @return Iterator, or NULL on error.
 */

#define aug_getseqfirst(this_) \
    (this_)->vtbl_->getfirst_(this_)

/**
 * Is empty test.
 * @param this_ The object.
 * @return True if empty.
 */

#define aug_isseqempty(this_) \
    (this_)->vtbl_->isempty_(this_)

/**
 * Get number of items.
 * @param this_ The object.
 * @return Number of items.
 */

#define aug_getseqsize(this_) \
    (this_)->vtbl_->getsize_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_seq> {
        typedef aug_seqvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_seqid;
        }
    };
}

namespace aug {

    template <typename T>
    class seq {

        aug_seq seq_;

        seq(const seq&);

        seq&
        operator =(const seq&);

        static void*
        cast_(aug_seq* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_seq* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_seq* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static aug_result
        getvalue_(aug_seq* this_, struct aug_var* value, const struct aug_var* key) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getseqvalue_(*value, *key);
        }
        static aug_iter*
        getfirst_(aug_seq* this_) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->getseqfirst_());
        }
        static int
        isempty_(aug_seq* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->isseqempty_();
        }
        static unsigned
        getsize_(aug_seq* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getseqsize_();
        }
        static const aug_seqvtbl*
        vtbl()
        {
            static const aug_seqvtbl local = {
                cast_,
                retain_,
                release_,
                getvalue_,
                getfirst_,
                isempty_,
                getsize_
            };
            return &local;
        }
    public:
        explicit
        seq(T* impl = 0)
        {
            this->seq_.vtbl_ = this->vtbl();
            this->seq_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->seq_.impl_ = impl;
        }
        aug_seq*
        get() AUG_NOTHROW
        {
            return &this->seq_;
        }
        operator aug::obref<aug_seq>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    typedef aug::obref<aug_seq> seqref;
    typedef aug::smartob<aug_seq> seqptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline void
    getseqvalue(aug::obref<aug_seq> this_, struct aug_var& value, const struct aug_var& key)
    {
        verify(this_.get()->vtbl_->getvalue_(this_.get(), &value, &key), this_);
    }

    inline aug::smartob<aug_iter>
    getseqfirst(aug::obref<aug_seq> this_) AUG_NOTHROW
    {
        return aug::object_attach<aug_iter>(this_.get()->vtbl_->getfirst_(this_.get()));
    }

    inline int
    isseqempty(aug::obref<aug_seq> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->isempty_(this_.get());
    }

    inline unsigned
    getseqsize(aug::obref<aug_seq> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getsize_(this_.get());
    }

    template <typename T>
    class seq_base {
        seq<T> seq_;
        int refs_;
    protected:
        ~seq_base()
        {
            /* Always deleted via derived, so no need to be virtual. */
        }
        seq_base()
            : refs_(1)
        {
            this->seq_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_seq>(id))
                return aug::object_retain<aug_object>(this->seq_);
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
        static aug::smartob<aug_seq>
        attach(T* ptr)
        {
            return aug::object_attach<aug_seq>(ptr->seq_);
        }
    };

    template <typename T>
    class scoped_seq_base {
        seq<T> seq_;
    protected:
        ~scoped_seq_base()
        {
        }
        scoped_seq_base()
        {
            this->seq_.reset(static_cast<T*>(this));
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_seq>(id))
                return aug::object_retain<aug_object>(this->seq_);
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
            return aug::obref<aug_object>(this->seq_).get();
        }
        aug_seq*
        get() AUG_NOTHROW
        {
            return this->seq_.get();
        }
        operator aug::obref<aug_seq>() AUG_NOTHROW
        {
            return this->seq_;
        }
    };

    template <typename T>
    class seq_wrapper {
        seq<seq_wrapper<T> > seq_;
        T impl_;
        int refs_;
        explicit
        seq_wrapper(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->seq_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_seq>(id))
                return aug::object_retain<aug_object>(this->seq_);
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
        getseqvalue_(struct aug_var& value, const struct aug_var& key) AUG_NOTHROW
        {
            return this->impl_.getseqvalue_(value, key);
        }
        aug::smartob<aug_iter>
        getseqfirst_() AUG_NOTHROW
        {
            return this->impl_.getseqfirst_();
        }
        int
        isseqempty_() AUG_NOTHROW
        {
            return this->impl_.isseqempty_();
        }
        unsigned
        getseqsize_() AUG_NOTHROW
        {
            return this->impl_.getseqsize_();
        }
        static aug::smartob<aug_seq>
        create(const T& impl = T())
        {
            seq_wrapper* ptr(new seq_wrapper(impl));
            return aug::object_attach<aug_seq>(ptr->seq_);
        }
    };

    template <typename T>
    class scoped_seq_wrapper {
        seq<scoped_seq_wrapper<T> > seq_;
        T impl_;
    public:
        explicit
        scoped_seq_wrapper(const T& impl = T())
            : impl_(impl)
        {
            this->seq_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_seq>(id))
                return aug::object_retain<aug_object>(this->seq_);
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
        getseqvalue_(struct aug_var& value, const struct aug_var& key) AUG_NOTHROW
        {
            return this->impl_.getseqvalue_(value, key);
        }
        aug::smartob<aug_iter>
        getseqfirst_() AUG_NOTHROW
        {
            return this->impl_.getseqfirst_();
        }
        int
        isseqempty_() AUG_NOTHROW
        {
            return this->impl_.isseqempty_();
        }
        unsigned
        getseqsize_() AUG_NOTHROW
        {
            return this->impl_.getseqsize_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->seq_).get();
        }
        aug_seq*
        get() AUG_NOTHROW
        {
            return this->seq_.get();
        }
        operator aug::obref<aug_seq>() AUG_NOTHROW
        {
            return this->seq_;
        }
    };

    typedef aug::smartob<aug_seq> seqptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_SEQ_H */
