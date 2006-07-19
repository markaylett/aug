/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_EXCEPTION_HPP
#define AUGSYSPP_EXCEPTION_HPP

#include "augsyspp/config.hpp"

#include "augsys/errinfo.h"

#include <stdexcept>

namespace aug {

    class errinfo_error : public std::runtime_error {
    public:
        explicit
        errinfo_error(const std::string& s)
            : std::runtime_error(s)
        {
        }
    };

    class local_error : public errinfo_error {
    public:
        explicit
        local_error(const std::string& s)
            : errinfo_error(s)
        {
        }
    };

    class system_error : public errinfo_error {
    public:
        explicit
        system_error(const std::string& s)
            : errinfo_error(s)
        {
        }
    };

    class posix_error : public system_error {
    public:
        explicit
        posix_error(const std::string& s)
            : system_error(s)
        {
        }
    };

    class win32_error : public system_error {
    public:
        explicit
        win32_error(const std::string& s)
            : system_error(s)
        {
        }
    };

    inline void
    throwerrinfo(const std::string& s)
    {
        switch (aug_errsrc) {
        case AUG_SRCLOCAL:
            throw local_error(s);
        case AUG_SRCPOSIX:
            throw posix_error(s);
        case AUG_SRCWIN32:
            throw win32_error(s);
        default:
            throw errinfo_error(s);
        }
    }
}

/** The following series of catch blocks would typically be used to contain
    exceptions from within functions that cannot throw. */

#define AUG_PERRINFOCATCH \
catch (const aug::errinfo_error& e) { \
    aug_perrinfo(e.what()); \
} catch (const std::exception& e) { \
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXCEPT, \
                   e.what()); \
    aug_perrinfo("std::exception"); \
} catch (...) { \
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXCEPT, \
                   "unknown error"); \
    aug_perrinfo("c++ exception"); \
} do { } while (0)

#define AUG_SETERRINFOCATCH \
catch (const aug::errinfo_error& e) { \
    aug_error(e.what()); \
} catch (const std::exception& e) { \
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXCEPT, \
                   e.what()); \
} catch (...) { \
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXCEPT, \
                   "unknown error"); \
} do { } while (0)

#endif // AUGSYSPP_EXCEPTION_HPP
