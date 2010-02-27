#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT([aug],[trunk],[mark.aylett@gmail.com])

AM_INIT_AUTOMAKE()
AC_CONFIG_HEADER([src/auglocal.h])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

LT_INIT([win32-dll])

# Multicast.

AC_ARG_ENABLE(multicast,
[  --enable-multicast      enable multicast support @<:@default=yes@:>@], [
    case x$enableval in
        xyes) enable_multicast=yes;;
        xno)  enable_multicast=no;;
        *) AC_MSG_ERROR(bad value $enableval for --enable-multicast);;
    esac
], [enable_multicast=yes])
AM_CONDITIONAL(ENABLE_MULTICAST, test x$enable_multicast = xyes)

if test x$enable_multicast = xyes; then
    AC_DEFINE(ENABLE_MULTICAST, 1, [Define to 1 for multicast support.])
fi

# SMP.

AC_ARG_ENABLE(smp,
[  --enable-smp            enable smp support @<:@default=yes@:>@], [
    case x$enableval in
        xyes) enable_smp=yes;;
        xno)  enable_smp=no;;
        *) AC_MSG_ERROR(bad value $enableval for --enable-smp);;
    esac
], [enable_smp=yes])
AM_CONDITIONAL(ENABLE_SMP, test x$enable_smp = xyes)

if test x$enable_smp = xyes; then
    AC_DEFINE(ENABLE_SMP, 1, [Define to 1 for smp support.])
fi

# Threads.

AC_ARG_ENABLE(threads,
[  --enable-threads        enable threads support @<:@default=yes@:>@], [
    case x$enableval in
        xyes) enable_threads=yes;;
        xno)  enable_threads=no;;
        *) AC_MSG_ERROR(bad value $enableval for --enable-threads);;
    esac
], [enable_threads=yes])
AM_CONDITIONAL(ENABLE_THREADS, test x$enable_threads = xyes)

if test x$enable_threads = xyes; then
    AC_DEFINE(ENABLE_THREADS, 1, [Define to 1 for threads support.])
fi

# CWeb.

AC_ARG_WITH(cweb,
[  --with-cweb             with cweb support @<:@default=no@:>@], [
    case x$withval in
        xyes) with_cweb=yes;;
        xno)  with_cweb=no;;
        *) AC_MSG_ERROR(bad value $withval for --with-cweb);;
    esac
], [with_cweb=no])
AM_CONDITIONAL(WITH_CWEB, test x$with_cweb = xyes)

# Python.

AC_ARG_WITH(python,
[  --with-python           with python support @<:@default=no@:>@], [
    case x$withval in
        xyes) with_python=yes;;
        xno)  with_python=no;;
        *) AC_MSG_ERROR(bad value $withval for --with-python);;
    esac
], [with_python=no])
AM_CONDITIONAL(WITH_PYTHON, test x$with_python = xyes)

if test x$with_python = xyes; then
    AC_DEFINE(WITH_PYTHON, 1, [Define to 1 for python support.])
    PY_CFLAGS=`src/sh/pyflags -c`
    AC_SUBST([PY_CFLAGS])
    PY_LDFLAGS=`src/sh/pyflags -l`
    AC_SUBST([PY_LDFLAGS])
fi

# Ruby.

AC_ARG_WITH(ruby,
[  --with-ruby             with ruby support @<:@default=no@:>@], [
    case x$withval in
        xyes) with_ruby=yes;;
        xno)  with_ruby=no;;
        *) AC_MSG_ERROR(bad value $withval for --with-ruby);;
    esac
], [with_ruby=no])
AM_CONDITIONAL(WITH_RUBY, test x$with_ruby = xyes)

if test x$with_ruby = xyes; then
    AC_DEFINE(WITH_RUBY, 1, [Define to 1 for ruby support.])
    RB_CFLAGS=`src/rb/rbflags -c`
    AC_SUBST([RB_CFLAGS])
    RB_LDFLAGS=`src/rb/rbflags -l`
    AC_SUBST([RB_LDFLAGS])
fi

# SSL.

AC_ARG_WITH(ssl,
[  --with-ssl              with ssl support @<:@default=yes@:>@], [
    case x$withval in
        xyes) with_ssl=yes;;
        xno)  with_ssl=no;;
        *)
            CFLAGS="-I$withval/include $CFLAGS"
            CPPFLAGS="-I$withval/include $CPPFLAGS"
            LDFLAGS="-L$withval/lib $LDFLAGS"
            with_ssl=yes
        ;;
    esac
], [with_ssl=yes])
AM_CONDITIONAL(WITH_SSL, test x$with_ssl = xyes)

if test x$with_ssl = xyes; then
    AC_DEFINE(WITH_SSL, 1, [Define to 1 for ssl support.])
fi

# Checks for libraries.

LDFLAGS="-lm $LDFLAGS"

if test x$enable_threads = xyes; then
   CPPFLAGS="-D_MT -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS $CPPFLAGS"
fi

case $host in
*-*-cygwin*|*-*-mingw*)

    AM_CONDITIONAL(AUG_WIN32, true)

    if test x$enable_multicast = xyes; then
        LIBS="-Wl,-liphlpapi $LIBS"
    fi

    if test x$enable_threads = xyes; then
        CPPFLAGS="-mthreads $CPPFLAGS"
        LDFLAGS="-mthreads $LDFLAGS"
    fi

    if test x$with_ssl = xyes; then
        AC_SEARCH_LIBS(SSL_library_init, [ssleay32])
        AC_SEARCH_LIBS(BIO_new, [eay32])
    fi

    AC_CHECK_TOOL(DLLWRAP, dllwrap, dllwrap)
    AC_CHECK_TOOL(WINDMS, windmc, windmc)
    AC_CHECK_TOOL(WINDRES, windres, windres)

    CPPFLAGS="-DWINVER=0x0501 -DFD_SETSIZE=256 -D__USE_W32_SOCKETS $CPPFLAGS"
    LIBS="-Wl,-lws2_32 -lws2_32 $LIBS"

    ;;
*)
    AM_CONDITIONAL(AUG_WIN32, false)

    if test x$enable_threads = xyes; then
        AC_SEARCH_LIBS(pthread_create, [pthread])
    fi

    if test x$with_ssl = xyes; then
        AC_SEARCH_LIBS(SSL_library_init, [ssl])
        AC_SEARCH_LIBS(BIO_new, [crypto])
    fi

    AC_SEARCH_LIBS(gethostbyname, nsl)
    AC_SEARCH_LIBS(gethostname, nsl)
    AC_SEARCH_LIBS(socket, socket)
    AC_SEARCH_LIBS(dlopen, [dl dlcompat])

    ;;
esac

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([dlfcn.h sys/sockio.h openssl/err.h openssl/ssl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN

AC_MSG_CHECKING(for socklen_t)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#if !defined(_WIN32)
# include <sys/socket.h>
# include <unistd.h>
#else /* _WIN32 */
# include <winsock2.h>
# include <ws2tcpip.h>
#endif /* _WIN32 */
]], [[socklen_t x;]])], [result=yes], [result=no]);

if test x$result = xyes; then
    AC_MSG_RESULT(yes)
else
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#if !defined(_WIN32)
# include <sys/socket.h>
#else /* _WIN32 */
# include <winsock2.h>
#endif /* _WIN32 */
int accept(int, struct sockaddr*, size_t*);
]], [[]])], [result=yes], [result=no]);

    if test x$result = xyes; then
        AC_MSG_RESULT(size_t)
        AC_DEFINE(socklen_t, size_t, [socklen_t size])
    else
        AC_MSG_RESULT(int)
        AC_DEFINE(socklen_t, int, [socklen_t size])
    fi
fi

AC_MSG_CHECKING(for ipv6)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
# include <sys/types.h>
#if !defined(_WIN32)
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
#else /* _WIN32 */
# include <winsock2.h>
# include <ws2tcpip.h>
#endif /* _WIN32 */
]], [[int x = IPV6_ADD_MEMBERSHIP;]])], [result=yes], [result=no]);

if test x$result = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_IPV6, 1, [Define to 1 if you have ipv6 support.])
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(whether the compiler supports __func__)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
const char*
func()
{
    return __func__;
}
]], [[]])], [result=yes], [result=no]);

if test x$result = xyes; then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
    AC_MSG_CHECKING(whether the compiler supports __FUNCTION__)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
const char*
func()
{
    return __FUNCTION__;
}
]], [[]])], [result=yes], [result=no]);

    if test x$result = xyes; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(__func__, __FUNCTION__,
            [Define to the equivalent of __func__])
    else
        AC_MSG_RESULT(no)
        AC_DEFINE(__func__, "unknown",
            [Define to the equivalent of __func__])
    fi
fi

# Checks for library functions.
AC_CHECK_FUNCS([bzero localtime_r poll strlcpy timegm])

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 doc/man/Makefile
                 etc/Makefile
                 src/Makefile
                 src/c/Makefile
                 src/c/augctx/Makefile
                 src/c/augctx/posix/Makefile
                 src/c/augctx/win32/Makefile
                 src/c/augext/Makefile
                 src/c/augmar/Makefile
                 src/c/augnet/Makefile
                 src/c/augres/Makefile
                 src/c/augserv/Makefile
                 src/c/augserv/posix/Makefile
                 src/c/augserv/win32/Makefile
                 src/c/augsys/Makefile
                 src/c/augsys/osx/Makefile
                 src/c/augsys/posix/Makefile
                 src/c/augsys/win32/Makefile
                 src/c/augutil/Makefile
                 src/c/augutil/posix/Makefile
                 src/c/augutil/win32/Makefile
                 src/c/mar/Makefile
                 src/cpp/Makefile
                 src/cpp/augctxpp/Makefile
                 src/cpp/augmarpp/Makefile
                 src/cpp/augnetpp/Makefile
                 src/cpp/augservpp/Makefile
                 src/cpp/augsyspp/Makefile
                 src/cpp/augutilpp/Makefile
                 src/cpp/augaspp/Makefile
                 src/cpp/augsubpp/Makefile
                 src/cpp/augd/Makefile
                 src/cpp/htdigest/Makefile
                 src/mod/Makefile
                 src/mod/aught/Makefile
                 src/mod/augpy/Makefile
                 src/mod/augrb/Makefile
                 src/mod/augsub/Makefile
                 src/pl/Makefile
                 src/py/Makefile
                 src/rb/Makefile
                 src/sh/Makefile
                 example/Makefile
                 example/augas/Makefile
                 example/augas/etc/Makefile
                 example/augas/htdocs/Makefile
                 example/cpp/Makefile
                 test/Makefile
                 vc7/Makefile
                 vc9/Makefile])
AC_OUTPUT
