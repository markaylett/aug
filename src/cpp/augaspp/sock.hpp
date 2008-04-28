/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SOCK_HPP
#define AUGRTPP_SOCK_HPP

#include "augaspp/object.hpp"

#include "augsyspp/smartfd.hpp"

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

        virtual autosd
        do_release() = 0;

        virtual sdref
        do_sd() const = 0;

        virtual sockstate
        do_state() const = 0;

    public:
        virtual
        ~sock_base() AUG_NOTHROW;

        autosd
        release()
        {
            return do_release();
        }
        sdref
        sd() const
        {
            return do_sd();
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
