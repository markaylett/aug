/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGNETPP_WRITER_HPP
#define AUGNETPP_WRITER_HPP

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/writer.h"

namespace aug {

    inline void
    appendwriter(aug_writer_t writer, blobref ref)
    {
        verify(aug_appendwriter(writer, ref.get()));
    }

    inline aug_rsize
    writesome(aug_writer_t writer, streamref ref)
    {
        return verify(aug_writesome(writer, ref.get()));
    }

    inline bool
    empty(aug_writer_t writer)
    {
        return aug_writerempty(writer) ? true : false;
    }

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
            return aug::empty(writer_);
        }

        aug_rsize
        size() const
        {
            return verify(aug_writersize(writer_));
        }
    };

    inline void
    swap(writer& lhs, writer& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }
}

inline bool
isnull(aug_writer_t writer)
{
    return !writer;
}

#endif // AUGNETPP_WRITER_HPP
