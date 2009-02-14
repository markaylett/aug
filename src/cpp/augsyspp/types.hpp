/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
