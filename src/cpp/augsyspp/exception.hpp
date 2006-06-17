/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_EXCEPTION_HPP
#define AUGSYSPP_EXCEPTION_HPP

#include "augsyspp/config.hpp"

#include "augsys/errno.h"  // aug_errno()
#include "augsys/string.h" // aug_strerror()

#include <stdexcept>

namespace aug {

    namespace detail {

        inline std::string
        makewhat(const std::string& s, int num)
        {
            std::string what(s);
            what += ": ";
            what += aug_strerror(num);
            return what;
        }
    }

    class AUGSYSPP_API posix_error : public std::runtime_error {
        const int num_;
    public:
        posix_error(const std::string& s)
            : std::runtime_error(detail::makewhat(s, aug_errno())),
              num_(aug_errno())
        {
        }

        posix_error(const std::string& s, int num)
            : std::runtime_error(detail::makewhat(s, num)),
              num_(num)
        {
        }

        int
        num() const NOTHROW
        {
            return num_;
        }
    };

    class AUGSYSPP_API intr_error : public posix_error {
    public:
        intr_error(const std::string& s)
            : posix_error(s, EINTR)
        {
        }
    };

    class AUGSYSPP_API null_error : public std::exception {
    public:
        const char*
        what() const NOTHROW
        {
            return "null exception";
        }
    };

    class AUGSYSPP_API timeout_error : public std::exception {
    public:
        const char*
        what() const NOTHROW
        {
            return "timeout exception";
        }
    };

    inline void
    error(const std::string& s, int num)
    {
        if (EINTR == num)
            throw intr_error(s);

        throw posix_error(s, num);
    }

    inline void
    error(const std::string& s)
    {
        error(s, aug_errno());
    }
}

#define AUG_CATCHRETURN \
catch (const aug::posix_error& e) { \
    aug_error(e.what()); \
    aug_seterrno(e.num()); \
} catch (const std::exception& e) { \
    aug_error(e.what()); \
    aug_seterrno(EINVAL); \
} catch (...) { \
    aug_error("unknown error"); \
    aug_seterrno(EINVAL); \
} \
return

#endif // AUGSYSPP_EXCEPTION_HPP
