/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_NULL_HPP
#define AUGSYSPP_NULL_HPP

#include "augsyspp/config.hpp"

const struct null_ { } null = null_();

template <typename typeT>
inline bool
operator ==(const typeT& lhs, const null_&)
{
    return isnull(lhs);
}

template <typename typeT>
inline bool
operator ==(const null_&, const typeT& rhs)
{
    return isnull(rhs);
}

template <typename typeT>
inline bool
operator !=(const typeT& lhs, const null_&)
{
    return !isnull(lhs);
}

template <typename typeT>
inline bool
operator !=(const null_&, const typeT& rhs)
{
    return !isnull(rhs);
}

#endif // AUGSYSPP_NULL_HPP
