/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_SMARTFD_HPP
#define AUGSYSPP_SMARTFD_HPP

#include "augsyspp/base.hpp"
#include "augsyspp/types.hpp"
#include "augsyspp/utility.hpp" // perrinfo()

#include <algorithm>            // swap()

namespace aug {

    class smartfd {
        fdref ref_;

        smartfd(fdref ref, bool retain) AUG_NOTHROW
            : ref_(ref)
        {
            if (null != ref && retain)
                retainfd(ref.get());
        }

    public:
        ~smartfd() AUG_NOTHROW
        {
            if (null != ref_ && -1 == aug_releasefd(ref_.get()))
                perrinfo("aug_releasefd() failed");
        }

        smartfd(const null_&) AUG_NOTHROW
            : ref_(null)
        {
        }

        smartfd(const smartfd& rhs)
            : ref_(rhs.ref_)
        {
            if (null != ref_)
                retainfd(ref_.get());
        }

        smartfd&
        operator =(const null_&) AUG_NOTHROW
        {
            *this = smartfd(null);
            return *this;
        }

        smartfd&
        operator =(const smartfd& rhs)
        {
            smartfd tmp(rhs);
            swap(tmp);
            return *this;
        }

        void
        swap(smartfd& rhs) AUG_NOTHROW
        {
            std::swap(ref_, rhs.ref_);
        }

        void
        release()
        {
            if (null != ref_) {
                fdref ref(ref_);
                ref_ = null;
                releasefd(ref.get());
            }
        }

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

        int
        get() const
        {
            return ref_.get();
        }

        operator fdref() const
        {
            return ref_;
        }
    };
}

inline bool
isnull(const aug::smartfd& sfd)
{
    return -1 == sfd.get();
}

#endif // AUGSYSPP_SMARTFD_HPP
