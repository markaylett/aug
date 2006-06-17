/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_EXCEPTION_HPP
#define AUGSYSPP_EXCEPTION_HPP

#include "augsyspp/config.hpp"

#include <stdexcept>

namespace aug {

    class AUGSYSPP_API posix_error : public std::runtime_error {
        const int num_;
    public:
        explicit
        posix_error(const std::string& s);

        posix_error(const std::string& s, int num);

        int
        num() const NOTHROW
        {
            return num_;
        }
    };

    class AUGSYSPP_API intr_error : public posix_error {
    public:
        explicit
        intr_error(const std::string& s);
    };

    class AUGSYSPP_API null_error : public std::exception {
    public:
        const char*
        what() const NOTHROW;
    };

    class AUGSYSPP_API timeout_error : public std::exception {
    public:
        const char*
        what() const NOTHROW;
    };

    AUGSYSPP_API void
    error(const std::string& s);

    AUGSYSPP_API void
    error(const std::string& s, int num);
}

#define AUG_CATCHRETURN \
catch (const aug::posix_error& e) { \
    aug_error(e.what()); \
    errno = e.num(); \
} catch (const std::exception& e) { \
    aug_error(e.what()); \
    errno = EINVAL; \
} catch (...) { \
    aug_error("unknown error"); \
    errno = EINVAL; \
} \
return

#endif // AUGSYSPP_EXCEPTION_HPP
