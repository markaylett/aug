/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_EXCEPTION_HPP
#define AUGSYSPP_EXCEPTION_HPP

#include "augsyspp/config.hpp"

#include "augsys/errinfo.h"

#include <stdexcept>

namespace aug {

    class AUGSYSPP_API error : public std::runtime_error {
    public:
        explicit
        error(const std::string& s)
            : std::runtime_error(s)
        {
        }
    };

    class AUGSYSPP_API local_error : public error {
    public:
        explicit
        local_error(const std::string& s)
            : error(s)
        {
        }
    };

    class AUGSYSPP_API system_error : public error {
    public:
        explicit
        system_error(const std::string& s)
            : error(s)
        {
        }
    };

    class AUGSYSPP_API posix_error : public system_error {
    public:
        explicit
        posix_error(const std::string& s)
            : system_error(s)
        {
        }
    };

    class AUGSYSPP_API win32_error : public system_error {
    public:
        explicit
        win32_error(const std::string& s)
            : system_error(s)
        {
        }
    };

    inline void
    throwerror(const std::string& s)
    {
        switch (aug_errsrc) {
        case AUG_SRCLOCAL:
            throw local_error(s);
        case AUG_SRCPOSIX:
            throw posix_error(s);
        case AUG_SRCWIN32:
            throw win32_error(s);
        default:
            throw error(s);
        }
    }
}

#define AUG_CATCHRETURN \
catch (const aug::error& e) { \
    aug_error(e.what()); \
} catch (const std::exception& e) { \
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXCEPT, \
                   e.what()); \
} catch (...) { \
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXCEPT, \
                   "unknown error"); \
} \
return

#endif // AUGSYSPP_EXCEPTION_HPP
