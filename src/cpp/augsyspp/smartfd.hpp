/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_SMARTFD_HPP
#define AUGSYSPP_SMARTFD_HPP

#include "augsyspp/types.hpp"

namespace aug {

    class AUGSYSPP_API smartfd {
        fdref ref_;

        explicit
        smartfd(fdref ref, bool retain) NOTHROW;

    public:
        ~smartfd() NOTHROW;

        smartfd(const null_&) NOTHROW
            : ref_(null)
        {
        }

        smartfd(const smartfd& rhs);

        smartfd&
        operator =(const smartfd& rhs);

        void
        swap(smartfd& rhs) NOTHROW;

        void
        release();

        static smartfd
        attach(int fd)
        {
            return smartfd(fd, false);
        }

        static smartfd
        retain(int fd)
        {
            return smartfd(fd, true);
        }

        operator fdref() const
        {
            return ref_;
        }
        int
        get() const
        {
            return ref_.get();
        }
    };
}

#endif // AUGSYSPP_SMARTFD_HPP
