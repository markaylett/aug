/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_TYPES_HPP
#define AUGSYSPP_TYPES_HPP

#include "augctxpp/types.hpp"

#include "augsys/types.h"

namespace aug {

    struct fd_traits {
        typedef aug_fd ref;
        static aug_fd
        bad() AUG_NOTHROW
        {
            return AUG_BADFD;
        }
        static int
        compare(aug_fd lhs, aug_fd rhs) AUG_NOTHROW
        {
            if (lhs < rhs)
                return -1;
            if (rhs < lhs)
                return 1;
            return 0;
        }
    };

    struct sd_traits {
        typedef aug_sd ref;
        static aug_sd
        bad() AUG_NOTHROW
        {
            return AUG_BADSD;
        }
        static int
        compare(aug_sd lhs, aug_sd rhs) AUG_NOTHROW
        {
            if (lhs < rhs)
                return -1;
            if (rhs < lhs)
                return 1;
            return 0;
        }
    };

    typedef basic_ref<fd_traits> fdref;
    typedef basic_ref<sd_traits> sdref;
    typedef basic_ref<sd_traits> mdref;
}

#endif // AUGSYSPP_TYPES_HPP
