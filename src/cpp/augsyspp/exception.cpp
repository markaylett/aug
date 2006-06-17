/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYSPP_BUILD
#include "augsyspp/exception.hpp"

#include "augsys/errno.h"
#include "augsys/string.h"

using namespace aug;
using namespace std;

namespace {

    string
    makewhat(const string& s, int num)
    {
        string what(s);
        what += ": ";
        what += aug_strerror(num);
        return what;
    }
}

AUGSYSPP_API
posix_error::posix_error(const string& s)
    : runtime_error(makewhat(s, errno)),
      num_(errno)
{
}

AUGSYSPP_API
posix_error::posix_error(const string& s, int num)
    : runtime_error(makewhat(s, num)),
      num_(num)
{
}

AUGSYSPP_API
intr_error::intr_error(const string& s)
    : posix_error(s, EINTR)
{
}

AUGSYSPP_API const char*
null_error::what() const NOTHROW
{
    return "null exception";
}

AUGSYSPP_API const char*
timeout_error::what() const NOTHROW
{
    return "timeout exception";
}

AUGSYSPP_API void
aug::error(const std::string& s)
{
    error(s, errno);
}

AUGSYSPP_API void
aug::error(const std::string& s, int num)
{
    if (EINTR == num)
        throw intr_error(s);

    throw posix_error(s, num);
}
