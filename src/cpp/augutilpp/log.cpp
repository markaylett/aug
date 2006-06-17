/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTILPP_BUILD
#include "augutilpp/log.hpp"

#include "augsyspp/exception.hpp"

#include "augsys/defs.h" /* AUG_BUFSIZE */

using namespace aug;
using namespace std;

AUGUTILPP_API void
aug::vformatlog(char* buf, size_t& n, int logLevel, const char* format,
                va_list args)
{
    if (-1 == aug_vformatlog(buf, &n, logLevel, format, args))
        error("aug_vformatlog() failed");
}

AUGUTILPP_API void
aug::formatlog(char* buf, size_t& n, int logLevel, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int ret(aug_vformatlog(buf, &n, logLevel, format, args));
    va_end(args);
    if (-1 == ret)
        error("aug_vformatlog() failed");
}

AUGUTILPP_API string
aug::vformatlog(int logLevel, const char* format, va_list args)
{
    char buf[AUG_BUFSIZE];
    size_t n(sizeof(buf));
    vformatlog(buf, n, logLevel, format, args);
    return string(buf, n);
}

AUGUTILPP_API string
aug::formatlog(int logLevel, const char* format, ...)
{
    char buf[AUG_BUFSIZE];
    size_t n(sizeof(buf));

    va_list args;
    va_start(args, format);
    int ret(aug_vformatlog(buf, &n, logLevel, format, args));
    va_end(args);
    if (-1 == ret)
        error("aug_vformatlog() failed");

    return string(buf, n);
}
