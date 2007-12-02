/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_LISTENER_HPP
#define AUGRTPP_LISTENER_HPP

#include "augaspp/sock.hpp"

#include "augsyspp.hpp"

namespace aug {

    class listener : public sock_base {

        sessionptr session_;
        maud_handle sock_;
        smartfd sfd_;

        maud_handle&
        do_get();

        const maud_handle&
        do_get() const;

        const sessionptr&
        do_session() const;

        smartfd
        do_sfd() const;

        sockstate
        do_state() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const sessionptr& session, void* user, const smartfd& sfd);
    };

    typedef smartptr<listener> listenerptr;
}

#endif // AUGRTPP_LISTENER_HPP
