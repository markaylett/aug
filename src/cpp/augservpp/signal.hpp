/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
