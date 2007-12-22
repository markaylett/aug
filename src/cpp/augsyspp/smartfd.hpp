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
            if (null != ref_ && -1 == aub_releasefd(ref_.get()))
                perrinfo("aub_releasefd() failed");
        }

        smartfd(const null_&) AUG_NOTHROW
            : ref_(null)
        {
        }

        smartfd(const smartfd& rhs) AUG_NOTHROW
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
        operator =(const smartfd& rhs) AUG_NOTHROW
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

        static smartfd
        attach(fdref ref)
        {
            return smartfd(ref, false);
        }

        static smartfd
        retain(fdref ref)
        {
            return smartfd(ref, true);
        }

        int
        get() const AUG_NOTHROW
        {
            return ref_.get();
        }

        operator fdref() const AUG_NOTHROW
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
