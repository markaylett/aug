/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_WRITER_HPP
#define AUGNETPP_WRITER_HPP

#include "augsyspp/exception.hpp"

#include "augnet/writer.h"

namespace aug {

    class writer {

        aug_writer_t writer_;

        writer(const writer&);

        writer&
        operator =(const writer&);

    public:
        ~writer() AUG_NOTHROW
        {
            if (-1 == aug_destroywriter(writer_))
                perrinfo("aug_destroywriter() failed");
        }

        writer()
            : writer_(aug_createwriter())
        {
            verify(writer_);
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
    };

    inline void
    appendbuf(aug_writer_t writer, const aug_var& var)
    {
        verify(aug_appendbuf(writer, &var));
    }

    inline ssize_t
    writesome(aug_writer_t writer, fdref ref)
    {
        return verify(aug_writesome(writer, ref.get()));
    }
}

#endif // AUGNETPP_WRITER_HPP
