/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_SMARTFD_HPP
#define AUGSYSPP_SMARTFD_HPP

#include "augsyspp/base.hpp"
#include "augsyspp/types.hpp"
#include "augsyspp/utility.hpp" // perrinfo()

#include <algorithm>            // swap()
#include <cassert>

namespace aug {

    template <typename traitsT>
    class autoclose {
        struct proxy {
            typename traitsT::ref ref_;
            explicit
            proxy(typename traitsT::ref ref)
                : ref_(ref)
            {
            }
        };
        typename traitsT::ref ref_;
    public:
        ~autoclose()
        {
            if (ref_ != traitsT::bad())
                traitsT::close(ref_);
        }
        explicit
        autoclose(typename traitsT::ref ref = traitsT::bad())
            : ref_(ref)
        {
        }
        autoclose(autoclose& rhs)
            : ref_(rhs.release())
        {
        }
        autoclose(const proxy& rhs)
            : ref_(rhs.ref_)
        {
        }
        autoclose&
        operator =(autoclose& rhs)
        {
            reset(rhs.release());
            return *this;
        }
        autoclose&
        operator =(const proxy& rhs)
        {
            reset(rhs.ref_);
            return *this;
        }
        void
        reset(typename traitsT::ref ref = traitsT::bad())
        {
            if (ref != ref_ && ref_ != traitsT::bad())
                traitsT::close(ref_);
            ref_ = ref;
        }
        typename traitsT::ref
        release()
        {
            typename traitsT::ref tmp(ref_);
            ref_ = traitsT::bad();
            return tmp;
        }
        operator proxy()
        {
            return proxy(release());
        }
        typename traitsT::ref
        get() const
        {
            return ref_;
        }
    };

    template <typename traitsT>
    class autoclose2 {
        struct proxy {
            typename traitsT::ref first_;
            typename traitsT::ref second_;
            proxy(typename traitsT::ref first, typename traitsT::ref second)
                : first_(first),
                  second_(second)
            {
            }
        };
        autoclose<traitsT> first_;
        autoclose<traitsT> second_;
    public:
        autoclose2(typename traitsT::ref first, typename traitsT::ref second)
            : first_(first),
              second_(second)
        {
        }
        autoclose2(autoclose2& rhs)
            : first_(rhs.first_),
              second_(rhs.second_)
        {
        }
        autoclose2(const proxy& rhs)
            : first_(rhs.first_),
              second_(rhs.second_)
        {
        }
        autoclose2&
        operator =(autoclose2& rhs)
        {
            first_ = rhs.first_;
            second_ = rhs.second_;
            return *this;
        }
        autoclose2&
        operator =(const proxy& rhs)
        {
            first_.reset(rhs.first_);
            second_.reset(rhs.second_);
            return *this;
        }
        operator proxy()
        {
            return proxy(first_.release(), second_.release());
        }
        autoclose<traitsT>&
        operator [](unsigned i)
        {
            assert(i < 2);
            return 0 == i ? first_ : second_;
        }
        const autoclose<traitsT>&
        operator [](unsigned i) const
        {
            assert(i < 2);
            return 0 == i ? first_ : second_;
        }
    };

    typedef autoclose<fd_traits> autofd;
    typedef autoclose<sd_traits> autosd;
    typedef autoclose<sd_traits> automd;

    typedef autoclose2<fd_traits> autofds;
    typedef autoclose2<sd_traits> autosds;
    typedef autoclose2<sd_traits> automds;
}

template <typename traitsT>
bool
isnull(const aug::autoclose<traitsT>& ac)
{
    return ac.get() == traitsT::bad();
}

#endif // AUGSYSPP_SMARTFD_HPP
