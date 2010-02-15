/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGCTXPP_EXCEPTION_HPP
#define AUGCTXPP_EXCEPTION_HPP

#include "augctxpp/errinfo.hpp"
#include "augctxpp/types.hpp"

#include "augctx/base.h"

#include "augext/err.h"

#include <cstring>          // memcpy()
#include <exception>

namespace aug {

    struct none_exception : std::exception {
        const char*
        what() const throw() // required by gcc.
        {
            return "aug::none_exception";
        }
    };

    struct intr_exception : std::exception {
        const char*
        what() const throw() // required by gcc.
        {
            return "aug::intr_exception";
        }
    };

    struct block_exception : std::exception {
        const char*
        what() const throw() // required by gcc.
        {
            return "aug::block_exception";
        }
    };

    class errinfo_error : public std::exception {
    public:
        typedef aug_errinfo ctype;
    private:
        aug_errinfo errinfo_;
    public:
        errinfo_error()
        {
            memcpy(&errinfo_, aug_tlerr, sizeof(errinfo_));
        }
        explicit
        errinfo_error(const aug_errinfo& errinfo)
        {
            memcpy(&errinfo_, &errinfo, sizeof(errinfo_));
        }
        errinfo_error(const char* file, int line, const char* src, int num,
                      const char* format, va_list args)
        {
            aug_vseterrinfo(&errinfo_, file, line, src, num, format, args);
        }
        errinfo_error(const char* file, int line, const char* src, int num,
                      const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            aug_vseterrinfo(&errinfo_, file, line, src, num, format, args);
            va_end(args);
        }
        const char*
        what() const throw() // required by gcc.
        {
            return errinfo_.desc_;
        }
        aug_errinfo&
        errinfo()
        {
            return errinfo_;
        }
        operator aug_errinfo& ()
        {
            return errinfo_;
        }
        aug_errinfo&
        errinfo(aug_errinfo& dst) const
        {
            aug_seterrinfo(&dst, errinfo_.file_, errinfo_.line_,
                           errinfo_.src_, errinfo_.num_, errinfo_.desc_);
            return dst;
        }
        const aug_errinfo&
        errinfo() const
        {
            return errinfo_;
        }
        operator const aug_errinfo& () const
        {
            return errinfo_;
        }
    };

    template <const char* (*T)(), typename baseT = errinfo_error>
    class basic_error : public baseT {
    public:
        basic_error()
        {
        }
        explicit
        basic_error(const aug_errinfo& errinfo)
            : baseT(errinfo)
        {
        }
        basic_error(const char* file, int line, int num, const char* format,
                    va_list args)
        {
            aug_vseterrinfo(cptr(*this), file, line, T(), num, format, args);
        }
        basic_error(const char* file, int line, int num, const char* format,
                    ...)
        {
            va_list args;
            va_start(args, format);
            aug_vseterrinfo(cptr(*this), file, line, T(), num, format, args);
            va_end(args);
        }
    };

    namespace detail {
        inline const char*
        aug_src()
        {
            return "aug";
        }
    }

    typedef basic_error<detail::aug_src> aug_error;

    class system_error : public errinfo_error {
    public:
        system_error()
        {
        }
        explicit
        system_error(const aug_errinfo& errinfo)
            : errinfo_error(errinfo)
        {
        }
    };

    class dlfcn_error : public system_error {
    public:
        dlfcn_error()
        {
        }
        explicit
        dlfcn_error(const aug_errinfo& errinfo)
            : system_error(errinfo)
        {
        }
        dlfcn_error(const char* file, int line, const char* desc)
        {
            aug_seterrinfo(cptr(*this), file, line, "dlfcn", 1, desc);
        }
    };

    class posix_error : public system_error {
    public:
        posix_error()
        {
        }
        explicit
        posix_error(const aug_errinfo& errinfo)
            : system_error(errinfo)
        {
        }
        posix_error(const char* file, int line, int err)
        {
            aug_setposixerrinfo(cptr(*this), file, line, err);
        }
    };

#if defined(_WIN32)
    class win32_error : public system_error {
    public:
        win32_error()
        {
        }
        explicit
        win32_error(const aug_errinfo& errinfo)
            : system_error(errinfo)
        {
        }
        win32_error(const char* file, int line, unsigned long err)
        {
            aug_setwin32errinfo(cptr(*this), file, line, err);
        }
    };
#endif // _WIN32

    inline void
    throwerror(const aug_errinfo& errinfo)
    {
        const char* src = errsrc(errinfo);
        switch (src[0]) {
        case 'a':
            if (0 == strcmp(src + 1, "ug"))
                throw aug_error(errinfo);
            break;
        case 'd':
            if (0 == strcmp(src + 1, "lfcn"))
                throw dlfcn_error(errinfo);
            break;
        case 'p':
            if (0 == strcmp(src + 1, "osix"))
                throw posix_error(errinfo);
            break;
        case 'w':
#if defined(_WIN32)
            if (0 == strcmp(src + 1, "in32"))
                throw win32_error(errinfo);
#endif // _WIN32
            break;
        }
        throw errinfo_error();
    }
    inline void
    throwerror()
    {
        throwerror(*aug_tlerr);
    }

    namespace detail {

        template <typename T>
        struct result_traits {
            static bool
            error(T result)
            {
                if (aug_issuccess(result))
                    return false;

                if (aug_isnone(result))
                    throw none_exception();

                if (aug_isintr(result))
                    throw intr_exception();

                if (aug_isblock(result))
                    throw block_exception();

                return true;
            }
        };

        template <typename T>
        struct result_traits<T*> {
            static bool
            error(T* result)
            {
                return 0 == result;
            }
        };

        template <>
        struct result_traits<bool> {
            static bool
            error(bool result)
            {
                return !result;
            }
       };
    }

    template <typename T>
    T
    verify(T result)
    {
        if (detail::result_traits<T>::error(result))
            throwerror();
        return result;
    }

    template <typename T, typename U>
    T
    verify(T result, obref<U> src)
    {
        if (detail::result_traits<T>::error(result)) {

            errptr ptr(object_cast<aug_err>(src));
            if (null == ptr)
                throwerror();

            aug_errinfo errinfo;
            copyerrinfo(ptr, errinfo);
            throwerror(errinfo);
        }
        return result;
    }
}

// The following series of catch blocks would typically be used to contain
// exceptions within functions that cannot throw.

#define AUG_PERRINFOCATCH                                               \
    catch (const aug::errinfo_error& e) {                               \
        e.errinfo(*aug_tlerr);                                          \
        aug_perrinfo(aug_tlx, "aug::errinfo_error", &e.errinfo());      \
    } catch (const std::exception& e) {                                 \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, e.what());                          \
        aug_perrinfo(aug_tlx, "std::exception", NULL);                  \
    } catch (...) {                                                     \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, "no description available");        \
        aug_perrinfo(aug_tlx, "unknown", NULL);                         \
    } do { } while (0)

#define AUG_SETERRINFOCATCH                                             \
    catch (const aug::errinfo_error& e) {                               \
        e.errinfo(*aug_tlerr);                                          \
    } catch (const std::exception& e) {                                 \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, e.what());                          \
    } catch (...) {                                                     \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, "no description available");        \
    } do { } while (0)

#endif // AUGCTXPP_EXCEPTION_HPP
