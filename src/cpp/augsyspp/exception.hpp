/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_EXCEPTION_HPP
#define AUGSYSPP_EXCEPTION_HPP

#include "augsyspp/errinfo.hpp"
#include "augsyspp/types.hpp"

#include "augsys/utility.h" // aug_perrinfo()

#include "augctx/base.h"

#include <cstring>          // memcpy()
#include <exception>

namespace aug {

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
        void
        seterrinfo() const
        {
            aug_seterrinfo(0, errinfo_.file_, errinfo_.line_, errinfo_.src_,
                           errinfo_.num_, errinfo_.desc_);
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

    class system_error : public errinfo_error { };

    class dlfcn_error : public system_error {
    public:
        dlfcn_error()
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
        win32_error(const char* file, int line, unsigned long err)
        {
            aug_setwin32errinfo(cptr(*this), file, line, err);
        }
    };
#endif // _WIN32

    inline void
    fail()
    {
        const char* src = errsrc(*aug_tlerr);
        switch (src[0]) {
        case 'a':
            if (0 == strcmp(src + 1, "ug"))
                throw aug_error();
            break;
        case 'd':
            if (0 == strcmp(src + 1, "lfcn"))
                throw dlfcn_error();
            break;
        case 'p':
            if (0 == strcmp(src + 1, "osix"))
                throw posix_error();
            break;
        case 'w':
#if defined(_WIN32)
            if (0 == strcmp(src + 1, "in32"))
                throw win32_error();
#endif // _WIN32
            break;
        }
        throw errinfo_error();
    }

    namespace detail {

        template <typename T>
        struct result_traits {
            static T
            verify(T result)
            {
                if (result < 0)
                    fail();
                return result;
            }
        };

        template <typename T>
        struct result_traits<T*> {
            static T*
            verify(T* result)
            {
                if (!result)
                    fail();
                return result;
            }
        };

        template <>
        struct result_traits<bool> {
            static bool
            verify(bool result)
            {
                if (!result)
                    fail();
                return result;
            }
        };
    }

    template <typename T>
    T
    verify(T result)
    {
        return detail::result_traits<T>::verify(result);
    }
}

/**
 * The following series of catch blocks would typically be used to contain
 * exceptions within functions that cannot throw.
 */

#define AUG_PERRINFOCATCH                                               \
    catch (const aug::errinfo_error& e) {                               \
        e.seterrinfo();                                                 \
        aug_perrinfo(&e.errinfo(), "aug::errinfo_error");               \
    } catch (const std::exception& e) {                                 \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, e.what());                          \
        aug_perrinfo(0, "std::exception");                              \
    } catch (...) {                                                     \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, "no description available");        \
        aug_perrinfo(0, "unknown");                                     \
    } do { } while (0)

#define AUG_SETERRINFOCATCH                                             \
    catch (const aug::errinfo_error& e) {                               \
        e.seterrinfo();                                                 \
    } catch (const std::exception& e) {                                 \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, e.what());                          \
    } catch (...) {                                                     \
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug",            \
                       AUG_EEXCEPT, "no description available");        \
    } do { } while (0)

#endif // AUGSYSPP_EXCEPTION_HPP



