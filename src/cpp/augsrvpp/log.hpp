/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_LOG_HPP
#define AUGSRVPP_LOG_HPP

#include "augsrvpp/config.hpp"

#include "augctxpp/exception.hpp"

#include "augsrv/log.h"

namespace aug {

    inline void
    openlog(const char* path)
    {
        verify(aug_openlog(path));
    }

    inline void
    setsrvlogger(const char* sname)
    {
        verify(aug_setsrvlogger(sname));
    }
}

#endif // AUGSRVPP_LOG_HPP
