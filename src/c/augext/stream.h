/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Fri Oct 24 22:36:50 GMT Daylight Time 2008 */

#ifndef AUGEXT_STREAM_H
#define AUGEXT_STREAM_H

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

AUG_INTERFACE(aug_stream);

/* Interface definitions. */

struct iovec;

/**
 * @defgroup aug_stream aug_stream
 *
 * @ingroup Object
 *
 * @{
 */

struct aug_streamvtbl {
    AUG_VTBL(aug_stream);
    aug_result (*shutdown_)(aug_stream*);
    aug_rsize (*read_)(aug_stream*, void*, size_t);
    aug_rsize (*readv_)(aug_stream*, const struct iovec*, int);
    aug_rsize (*write_)(aug_stream*, const void*, size_t);
    aug_rsize (*writev_)(aug_stream*, const struct iovec*, int);
};

#define aug_shutdown(this_) \
    (this_)->vtbl_->shutdown_(this_)

#define aug_read(this_, buf, size) \
    (this_)->vtbl_->read_(this_, buf, size)

#define aug_readv(this_, iov, size) \
    (this_)->vtbl_->readv_(this_, iov, size)

#define aug_write(this_, buf, size) \
    (this_)->vtbl_->write_(this_, buf, size)

#define aug_writev(this_, iov, size) \
    (this_)->vtbl_->writev_(this_, iov, size)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_stream> {
        typedef aug_streamvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_streamid;
        }
    };
}

namespace aug {

    template <typename T>
    class stream {

        aug_stream stream_;

        stream(const stream&);

        stream&
        operator =(const stream&);

        static void*
        cast_(aug_stream* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_stream* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_stream* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static aug_result
        shutdown_(aug_stream* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->shutdown_();
        }
        static aug_rsize
        read_(aug_stream* this_, void* buf, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->read_(buf, size);
        }
        static aug_rsize
        readv_(aug_stream* this_, const struct iovec* iov, int size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->readv_(iov, size);
        }
        static aug_rsize
        write_(aug_stream* this_, const void* buf, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->write_(buf, size);
        }
        static aug_rsize
        writev_(aug_stream* this_, const struct iovec* iov, int size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->writev_(iov, size);
        }
        static const aug_streamvtbl*
        vtbl()
        {
            static const aug_streamvtbl local = {
                cast_,
                retain_,
                release_,
                shutdown_,
                read_,
                readv_,
                write_,
                writev_
            };
            return &local;
        }
    public:
        explicit
        stream(T* impl = 0)
        {
            this->stream_.vtbl_ = this->vtbl();
            this->stream_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->stream_.impl_ = impl;
        }
        aug_stream*
        get() AUG_NOTHROW
        {
            return &this->stream_;
        }
        operator aug::obref<aug_stream>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    typedef aug::obref<aug_stream> streamref;
    typedef aug::smartob<aug_stream> streamptr;
}
#endif /* __cplusplus */

/* C++ definitions. */

#if defined(__cplusplus)
namespace aug {

    inline void
    shutdown(aug::obref<aug_stream> this_)
    {
        verify(this_.get()->vtbl_->shutdown_(this_.get()), this_);
    }

    inline size_t
    read(aug::obref<aug_stream> this_, void* buf, size_t size)
    {
        return static_cast<size_t>(AUG_RESULT(verify(this_.get()->vtbl_->read_(this_.get(), buf, size), this_)));
    }

    inline size_t
    readv(aug::obref<aug_stream> this_, const struct iovec* iov, int size)
    {
        return static_cast<size_t>(AUG_RESULT(verify(this_.get()->vtbl_->readv_(this_.get(), iov, size), this_)));
    }

    inline size_t
    write(aug::obref<aug_stream> this_, const void* buf, size_t size)
    {
        return static_cast<size_t>(AUG_RESULT(verify(this_.get()->vtbl_->write_(this_.get(), buf, size), this_)));
    }

    inline size_t
    writev(aug::obref<aug_stream> this_, const struct iovec* iov, int size)
    {
        return static_cast<size_t>(AUG_RESULT(verify(this_.get()->vtbl_->writev_(this_.get(), iov, size), this_)));
    }

    template <typename T>
    class basic_stream {
        stream<basic_stream<T> > stream_;
        T impl_;
        int refs_;
        explicit
        basic_stream(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->stream_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_stream>(id))
                return aug::object_retain<aug_object>(this->stream_);
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
        void
        shutdown_() AUG_NOTHROW
        {
            return this->impl_.shutdown_();
        }
        size_t
        read_(void* buf, size_t size) AUG_NOTHROW
        {
            return this->impl_.read_(buf, size);
        }
        size_t
        readv_(const struct iovec* iov, int size) AUG_NOTHROW
        {
            return this->impl_.readv_(iov, size);
        }
        size_t
        write_(const void* buf, size_t size) AUG_NOTHROW
        {
            return this->impl_.write_(buf, size);
        }
        size_t
        writev_(const struct iovec* iov, int size) AUG_NOTHROW
        {
            return this->impl_.writev_(iov, size);
        }
        static aug::smartob<aug_stream>
        create(const T& impl = T())
        {
            basic_stream* ptr(new basic_stream(impl));
            return aug::object_attach<aug_stream>(ptr->stream_);
        }
    };

    template <typename T>
    class scoped_stream {
        stream<scoped_stream<T> > stream_;
        T impl_;
    public:
        explicit
        scoped_stream(const T& impl = T())
            : impl_(impl)
        {
            this->stream_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_stream>(id))
                return aug::object_retain<aug_object>(this->stream_);
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
        void
        shutdown_() AUG_NOTHROW
        {
            return this->impl_.shutdown_();
        }
        size_t
        read_(void* buf, size_t size) AUG_NOTHROW
        {
            return this->impl_.read_(buf, size);
        }
        size_t
        readv_(const struct iovec* iov, int size) AUG_NOTHROW
        {
            return this->impl_.readv_(iov, size);
        }
        size_t
        write_(const void* buf, size_t size) AUG_NOTHROW
        {
            return this->impl_.write_(buf, size);
        }
        size_t
        writev_(const struct iovec* iov, int size) AUG_NOTHROW
        {
            return this->impl_.writev_(iov, size);
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->stream_).get();
        }
        aug_stream*
        get() AUG_NOTHROW
        {
            return this->stream_.get();
        }
        operator aug::obref<aug_stream>() AUG_NOTHROW
        {
            return this->stream_;
        }
    };

    typedef aug::smartob<aug_stream> streamptr;
}
#endif /* __cplusplus */

#endif /* AUGEXT_STREAM_H */
