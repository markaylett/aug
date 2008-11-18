/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERVPP_LOG_HPP
#define AUGSERVPP_LOG_HPP

#include "augservpp/config.hpp"

#include "augctxpp/exception.hpp"

#include "augserv/log.h"

namespace aug {

    inline void
    openlog(const char* path)
    {
        verify(aug_openlog(path));
    }

    inline void
    setservlogger(const char* sname)
    {
        verify(aug_setservlogger(sname));
    }
}

#endif // AUGSERVPP_LOG_HPP
