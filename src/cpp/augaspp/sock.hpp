/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SOCK_HPP
#define AUGRTPP_SOCK_HPP

#include "augaspp/object.hpp"

namespace aug {

    enum sockstate {
        HANDSHAKE,
        CONNECTED,
        LISTENING,
        TEARDOWN,
        SHUTDOWN,
        CLOSED
    };

    class sock_base : public object_base {

        virtual channelobptr
        do_channelob() const = 0;

        virtual sockstate
        do_state() const = 0;

    public:
        virtual
        ~sock_base() AUG_NOTHROW;

        channelobptr
        channelob() const
        {
            return do_channelob();
        }
        sockstate
        state() const
        {
            return do_state();
        }
    };

    typedef smartptr<sock_base> sockptr;
}

#endif // AUGRTPP_SOCK_HPP
