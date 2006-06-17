/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_LOG_HPP
#define AUGSRVPP_LOG_HPP

#include "augsrvpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augsrv/log.h"

namespace aug {

    inline void
    openlog(const char* path)
    {
        if (-1 == aug_openlog(path))
            throwerror("aug_openlog() failed");
    }

    inline void
    setsrvlogger(const char* sname)
    {
        if (-1 == aug_setsrvlogger(sname))
            throwerror("aug_setsrvlogger() failed");
    }

    inline void
    unsetsrvlogger()
    {
        if (-1 == aug_unsetsrvlogger())
            throwerror("aug_unsetsrvlogger() failed");
    }
}

#endif // AUGSRVPP_LOG_HPP
