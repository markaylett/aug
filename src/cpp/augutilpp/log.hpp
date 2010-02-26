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
#ifndef AUGUTILPP_LOG_HPP
#define AUGUTILPP_LOG_HPP

#include "augutilpp/config.hpp"

#include "augctxpp/exception.hpp"

#include "augutil/log.h"

#include "augctx/defs.h" // AUG_MAXLINE

#include <string>

namespace aug {

    inline void
    vformatlog(char* buf, size_t& n, clockref clock, unsigned level,
               const char* format, va_list args)
    {
        verify(aug_vformatlog(buf, &n, clock.get(), level, format, args));
    }

    inline void
    formatlog(char* buf, size_t& n, clockref clock, unsigned level,
              const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        aug_result result(aug_vformatlog(buf, &n, clock.get(), level, format,
                                         args));
        va_end(args);
        verify(result);
    }

    inline std::string
    vformatlog(clockref clock, unsigned level, const char* format,
               va_list args)
    {
        char buf[AUG_MAXLINE];
        size_t n(sizeof(buf));
        vformatlog(buf, n, clock, level, format, args);
        return std::string(buf, n);
    }

    inline std::string
    formatlog(clockref clock, unsigned level, const char* format, ...)
    {
        char buf[AUG_MAXLINE];
        size_t n(sizeof(buf));

        va_list args;
        va_start(args, format);
        aug_result result(aug_vformatlog(buf, &n, clock.get(), level, format,
                                         args));
        va_end(args);
        verify(result);

        return std::string(buf, n);
    }

    inline logptr
    createdaemonlog(mpoolref mpool, clockref clock)
    {
        return object_attach<aug_log>
            (aug_createdaemonlog(mpool.get(), clock.get()));
    }

    inline void
    setdaemonlog(ctxref ctx)
    {
        verify(aug_setdaemonlog(ctx.get()));
    }
}

#endif // AUGUTILPP_LOG_HPP
