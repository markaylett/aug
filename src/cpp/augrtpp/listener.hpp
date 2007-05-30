/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_LISTENER_HPP
#define AUGRTPP_LISTENER_HPP

#include "augrtpp/sock.hpp"

#include "augsyspp.hpp"

namespace aug {

    class listener : public sock_base {

        servptr serv_;
        augrt_object sock_;
        smartfd sfd_;

        augrt_object&
        do_get();

        const augrt_object&
        do_get() const;

        const servptr&
        do_serv() const;

        smartfd
        do_sfd() const;

        sockstate
        do_state() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const servptr& serv, void* user, const smartfd& sfd);
    };

    typedef smartptr<listener> listenerptr;
}

#endif // AUGRTPP_LISTENER_HPP