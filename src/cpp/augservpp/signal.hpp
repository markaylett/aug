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
    sigblock()
    {
        verify(aug_sigblock());
    }

    inline void
    sigunblock()
    {
        verify(aug_sigunblock());
    }

    class scoped_sighandler {

        scoped_sighandler(const scoped_sighandler& rhs);

        scoped_sighandler&
        operator =(const scoped_sighandler& rhs);

    public:
        ~scoped_sighandler() AUG_NOTHROW
        {
            // Restore default handlers.

            if (aug_setsighandler(0) < 0)
                perrinfo(aug_tlx, "aug_setsighandler() failed");
        }

        scoped_sighandler(void (*handler)(int))
        {
            setsighandler(handler);
        }
    };

    class scoped_sigblock {

        scoped_sigblock(const scoped_sigblock& rhs);

        scoped_sigblock&
        operator =(const scoped_sigblock& rhs);

    public:
        ~scoped_sigblock() AUG_NOTHROW
        {
            if (aug_sigunblock() < 0)
                perrinfo(aug_tlx, "aug_sigunblock() failed");
        }

        scoped_sigblock()
        {
            sigblock();
        }
    };

    class scoped_sigunblock {

        scoped_sigunblock(const scoped_sigunblock& rhs);

        scoped_sigunblock&
        operator =(const scoped_sigunblock& rhs);

    public:
        ~scoped_sigunblock() AUG_NOTHROW
        {
            if (aug_sigblock() < 0)
                perrinfo(aug_tlx, "aug_sigblock() failed");
        }

        scoped_sigunblock()
        {
            sigunblock();
        }
    };
}

#endif // AUGSERVPP_SIGNAL_HPP
