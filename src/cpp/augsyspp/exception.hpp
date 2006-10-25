/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_EXCEPTION_HPP
#define AUGSYSPP_EXCEPTION_HPP

#include "augsyspp/errinfo.hpp"

#include <cstring> // memcpy()
#include <exception>

namespace aug {

    class errinfo_error : public std::exception {
    public:
        typedef struct aug_errinfo ctype;
    protected:
        struct aug_errinfo errinfo_;
    public:
        errinfo_error()
        {
            memcpy(&errinfo_, aug_geterrinfo(), sizeof(errinfo_));
        }
        errinfo_error(const char* file, int line, int src, int num,
                      const char* format, va_list args)
        {
            aug_vseterrinfo(&errinfo_, file, line, src, num, format, args);
        }
        errinfo_error(const char* file, int line, int src, int num,
                      const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            aug_vseterrinfo(&errinfo_, file, line, src, num, format, args);
            va_end(args);
        }
        const char*
        what() const AUG_NOTHROW
        {
            return errinfo_.desc_;
        }
        void
        seterrinfo() const
        {
            aug_seterrinfo(0, errinfo_.file_, errinfo_.line_, errinfo_.src_,
                           errinfo_.num_, errinfo_.desc_);
        }
        const struct aug_errinfo&
        errinfo() const
        {
            return errinfo_;
        }
        operator const struct aug_errinfo& () const
        {
            return errinfo_;
        }
    };

    template <int srcT, typename baseT = errinfo_error>
    class basic_error : public baseT {
    public:
        basic_error()
        {
        }
        basic_error(const char* file, int line, int num, const char* format,
                    va_list args)
        {
            aug_vseterrinfo(&static_cast<errinfo_error*>(this)->errinfo_,
                            file, line, srcT, num, format, args);
        }
        basic_error(const char* file, int line, int num, const char* format,
                    ...)
        {
            va_list args;
            va_start(args, format);
            aug_vseterrinfo(&static_cast<errinfo_error*>(this)->errinfo_,
                            file, line, srcT, num, format, args);
            va_end(args);
        }
    };

    class local_error : public errinfo_error {
    public:
        local_error()
        {
        }
        local_error(const char* file, int line, int num, const char* format,
                    va_list args)
        {
            aug_vseterrinfo(&errinfo_, file, line, AUG_SRCLOCAL, num, format,
                            args);
        }
        local_error(const char* file, int line, int num, const char* format,
                    ...)
        {
            va_list args;
            va_start(args, format);
            aug_vseterrinfo(&errinfo_, file, line, AUG_SRCLOCAL, num, format,
                            args);
            va_end(args);
        }
    };

    class system_error : public errinfo_error { };

    class posix_error : public system_error {
    public:
        posix_error()
        {
        }
        posix_error(const char* file, int line, int err)
        {
            aug_setposixerrinfo(&errinfo_, file, line, err);
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
            aug_setwin32errinfo(&errinfo_, file, line, err);
        }
    };
#endif // _WIN32

    class dlfcn_error : public system_error {
    public:
        dlfcn_error()
        {
        }
        dlfcn_error(const char* file, int line, const char* desc)
        {
            aug_seterrinfo(&errinfo_, file, line, AUG_SRCDLFCN, 1, desc);
        }
    };

    inline void
    fail()
    {
        switch (aug_errsrc) {
        case AUG_SRCLOCAL:
            throw local_error();
        case AUG_SRCPOSIX:
            throw posix_error();
        case AUG_SRCWIN32:
            throw win32_error();
        case AUG_SRCDLFCN:
            throw dlfcn_error();
        default:
            throw errinfo_error();
        }
    }

    template <typename T>
    T
    verify(T result)
    {
        if (-1 == result)
            fail();
        return result;
    }

    template <typename T>
    T*
    verify(T* result)
    {
        if (!result)
            fail();
        return result;
    }

    template <typename T>
    const T*
    verify(const T* result)
    {
        if (!result)
            fail();
        return result;
    }
}

/**
   The following series of catch blocks would typically be used to contain
   exceptions within functions that cannot throw.
*/

#define AUG_PERRINFOCATCH                                               \
    catch (const aug::errinfo_error& e) {                               \
        e.seterrinfo();                                                 \
        aug_perrinfo(aug::cptr(e), "aug::errinfo_error");               \
    } catch (const std::exception& e) {                                 \
        aug_seterrinfo(0, __FILE__, __LINE__, AUG_SRCLOCAL,             \
                       AUG_ECXX, e.what());                             \
        aug_perrinfo(0, "std::exception");                              \
    } catch (...) {                                                     \
        aug_seterrinfo(0, __FILE__, __LINE__, AUG_SRCLOCAL,             \
                       AUG_ECXX, "no description available");           \
        aug_perrinfo(0, "unknown");                                     \
    } do { } while (0)

#define AUG_SETERRINFOCATCH                                             \
    catch (const aug::errinfo_error& e) {                               \
        e.seterrinfo();                                                      \
    } catch (const std::exception& e) {                                 \
        aug_seterrinfo(0, __FILE__, __LINE__, AUG_SRCLOCAL,             \
                       AUG_ECXX, e.what());                             \
    } catch (...) {                                                     \
        aug_seterrinfo(0, __FILE__, __LINE__, AUG_SRCLOCAL,             \
                       AUG_ECXX, "no description available");           \
    } do { } while (0)

#endif // AUGSYSPP_EXCEPTION_HPP
