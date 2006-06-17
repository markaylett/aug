/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_LOG_HPP
#define AUGUTILPP_LOG_HPP

#include "augutilpp/config.hpp"

#include "augutil/log.h"

#include <string>

namespace aug {

    AUGUTILPP_API void
    vformatlog(char* buf, size_t& n, int logLevel, const char* format,
               va_list args);

    AUGUTILPP_API void
    formatlog(char* buf, size_t& n, int logLevel, const char* format, ...);

    AUGUTILPP_API std::string
    vformatlog(int logLevel, const char* format, va_list args);

    AUGUTILPP_API std::string
    formatlog(int logLevel, const char* format, ...);
}

#endif // AUGUTILPP_LOG_HPP
