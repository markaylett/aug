/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMARPP_BUILD
#include "augmarpp/smartmar.hpp"

#include "augsyspp/exception.hpp"

#include "augsys/string.h" // aug_perror()

#include <algorithm>       // swap()

using namespace aug;
using namespace std;

AUGMARPP_API
smartmar::smartmar(marref ref, bool retain)
: ref_(ref)
{
    if (retain && null != ref) {

        if (-1 == aug_retainmar(ref.get()))
            error("aug_retainmar() failed");
    }
}

AUGMARPP_API
smartmar::~smartmar() NOTHROW
{
    if (null == ref_)
        return;

    if (null != ref_ && -1 == aug_releasemar(ref_.get()))
        aug_perror("aug_releasemar() failed");
}

AUGMARPP_API
smartmar::smartmar(const smartmar& rhs)
    : ref_(rhs.ref_)
{
    if (null != ref_ && -1 == aug_retainmar(ref_.get()))
        error("aug_retainmar() failed");
}

AUGMARPP_API smartmar&
smartmar::operator =(const smartmar& rhs)
{
    smartmar tmp(rhs);
    swap(tmp);
    return *this;
}

AUGMARPP_API void
smartmar::swap(smartmar& rhs) NOTHROW
{
    std::swap(ref_, rhs.ref_);
}

AUGMARPP_API void
smartmar::release()
{
    if (null != ref_) {
        marref ref(ref_);
        ref_ = null;
        if (-1 == aug_releasemar(ref.get()))
            error("aug_releasemar() failed");
    }
}
