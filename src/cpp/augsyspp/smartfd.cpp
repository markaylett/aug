/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYSPP_BUILD
#include "augsyspp/smartfd.hpp"

#include "augsyspp/base.hpp"

#include "augsys/string.h" // aug_perror()

#include <algorithm>       // swap()

using namespace aug;
using namespace std;

AUGSYSPP_API
smartfd::smartfd(fdref ref, bool retain) NOTHROW
: ref_(ref)
{
    if (retain && null != ref)
        retainfd(ref.get());
}

AUGSYSPP_API
smartfd::~smartfd() NOTHROW
{
    if (null != ref_ && -1 == aug_releasefd(ref_.get()))
        aug_perror("aug_releasefd() failed");
}

AUGSYSPP_API
smartfd::smartfd(const smartfd& rhs)
    : ref_(rhs.ref_)
{
    if (null != ref_)
        retainfd(ref_.get());
}

AUGSYSPP_API smartfd&
smartfd::operator =(const smartfd& rhs)
{
    smartfd tmp(rhs);
    swap(tmp);
    return *this;
}

AUGSYSPP_API void
smartfd::swap(smartfd& rhs) NOTHROW
{
    std::swap(ref_, rhs.ref_);
}

AUGSYSPP_API void
smartfd::release()
{
    if (null != ref_) {
        fdref ref(ref_);
        ref_ = null;
        releasefd(ref.get());
    }
}
