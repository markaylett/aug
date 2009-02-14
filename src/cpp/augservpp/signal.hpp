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
#ifndef AUGSERVPP_SIGNAL_HPP
#define AUGSERVPP_SIGNAL_HPP

#include "augservpp/config.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/utility.hpp" // perrinfo()

#include "augserv/signal.h"

namespace aug {

    inline void
    setsighandler(void (*handler)(int))
    {
        verify(aug_setsighandler(handler));
    }

    inline void
    blocksignals()
    {
        verify(aug_blocksignals());
    }

    inline void
    unblocksignals()
    {
        verify(aug_unblocksignals());
    }

    class scoped_block {

        scoped_block(const scoped_block& rhs);

        scoped_block&
        operator =(const scoped_block& rhs);

    public:
        ~scoped_block() AUG_NOTHROW
        {
            if (AUG_ISFAIL(aug_unblocksignals()))
                perrinfo(aug_tlx, "aug_unblocksignals() failed");
        }

        scoped_block()
        {
            blocksignals();
        }
    };

    class scoped_unblock {

        scoped_unblock(const scoped_unblock& rhs);

        scoped_unblock&
        operator =(const scoped_unblock& rhs);

    public:
        ~scoped_unblock() AUG_NOTHROW
        {
            if (AUG_ISFAIL(aug_blocksignals()))
                perrinfo(aug_tlx, "aug_blocksignals() failed");
        }

        scoped_unblock()
        {
            unblocksignals();
        }
    };
}

#endif // AUGSERVPP_SIGNAL_HPP
