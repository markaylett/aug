/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_WRITER_HPP
#define AUGNETPP_WRITER_HPP

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/writer.h"

namespace aug {

    class writer : public mpool_ops {

        aug_writer_t writer_;

        writer(const writer&);

        writer&
        operator =(const writer&);

    public:
        ~writer() AUG_NOTHROW
        {
            if (writer_)
                aug_destroywriter(writer_);
        }

        writer(const null_&) AUG_NOTHROW
           : writer_(0)
        {
        }

        explicit
        writer(mpoolref mpool)
            : writer_(aug_createwriter(mpool.get()))
        {
            verify(writer_);
        }

        void
        swap(writer& rhs) AUG_NOTHROW
        {
            std::swap(writer_, rhs.writer_);
        }

        operator aug_writer_t()
        {
            return writer_;
        }

        aug_writer_t
        get()
        {
            return writer_;
        }

        bool
        empty() const
        {
            return aug_writerempty(writer_) ? true : false;
        }

        size_t
        size() const
        {
            return AUG_RESULT(verify(aug_writersize(writer_)));
        }
    };

    inline void
    swap(writer& lhs, writer& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    inline void
    appendwriter(aug_writer_t writer, blobref ref)
    {
        verify(aug_appendwriter(writer, ref.get()));
    }

    inline size_t
    writesome(aug_writer_t writer, streamref ref)
    {
        return AUG_RESULT(verify(aug_writesome(writer, ref.get())));
    }
}

#endif // AUGNETPP_WRITER_HPP
