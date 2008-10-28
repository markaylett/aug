/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_SOCK_HPP
#define AUGASPP_SOCK_HPP

#include "augaspp/object.hpp"

#include "augext/chan.h"

namespace aug {

    enum sockstate {
        CONNECTING,
        ESTABLISHED,
        LISTENING,
        TEARDOWN,
        SHUTDOWN,
        CLOSED
    };

    class sock_base : public object_base {

        virtual void
        do_shutdown(chanref chan, unsigned flags, const timeval& now) = 0;

        virtual sockstate
        do_state() const = 0;

    public:
        virtual
        ~sock_base() AUG_NOTHROW;

        void
        shutdown(chanref chan, unsigned flags, const timeval& now)
        {
            do_shutdown(chan, flags, now);
        }
        sockstate
        state() const
        {
            return do_state();
        }
    };

    typedef smartptr<sock_base> sockptr;
}

#endif // AUGASPP_SOCK_HPP
