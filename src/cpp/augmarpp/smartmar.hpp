/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file smartmar.hpp
   \brief TODO
 */

#ifndef AUGMARPP_SMARTMAR_HPP
#define AUGMARPP_SMARTMAR_HPP

#include "augmarpp/types.hpp"

#include "augsyspp/exception.hpp"

#include <algorithm> // swap()

namespace aug {

    class smartmar {
        marref ref_;

        smartmar(marref ref, bool retain)
            : ref_(ref)
        {
            if (retain && null != ref) {

                if (-1 == aug_retainmar(ref.get()))
                    throwerrinfo("aug_retainmar() failed");
            }
        }

    public:
        ~smartmar() NOTHROW
        {
            if (null == ref_)
                return;

            if (null != ref_ && -1 == aug_releasemar(ref_.get()))
                aug_perrinfo("aug_releasemar() failed");
        }

        smartmar(const null_&) NOTHROW
        : ref_(null)
        {
        }

        smartmar(const smartmar& rhs)
            : ref_(rhs.ref_)
        {
            if (null != ref_ && -1 == aug_retainmar(ref_.get()))
                throwerrinfo("aug_retainmar() failed");
        }

        smartmar&
        operator =(const null_& rhs) NOTHROW
        {
            ref_ = null;
            return *this;
        }

        smartmar&
        operator =(const smartmar& rhs)
        {
            smartmar tmp(rhs);
            swap(tmp);
            return *this;
        }

        void
        swap(smartmar& rhs) NOTHROW
        {
            std::swap(ref_, rhs.ref_);
        }

        void
        release()
        {
            if (null != ref_) {
                marref ref(ref_);
                ref_ = null;
                if (-1 == aug_releasemar(ref.get()))
                    throwerrinfo("aug_releasemar() failed");
            }
        }

        static smartmar
        attach(aug_mar_t mar)
        {
            return smartmar(mar, false);
        }

        static smartmar
        retain(aug_mar_t mar)
        {
            return smartmar(mar, true);
        }

        operator marref() const
        {
            return ref_;
        }

        aug_mar_t
        get() const
        {
            return ref_.get();
        }
    };
}

inline bool
isnull(const aug::smartmar& mar)
{
    return 0 == mar.get();
}

#endif // AUGMARPP_SMARTMAR_HPP
