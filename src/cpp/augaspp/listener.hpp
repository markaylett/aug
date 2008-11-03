/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_LISTENER_HPP
#define AUGASPP_LISTENER_HPP

#include "augaspp/sock.hpp"

#include "augsyspp.hpp"

namespace aug {

    class listener : public sock_base {

        sessionptr session_;
        mod_handle sock_;
        sockstate state_;

        // object_base.

        mod_handle&
        do_get();

        const mod_handle&
        do_get() const;

        const sessionptr&
        do_session() const;

        // sock_base.

        void
        do_error(const char* desc);

        void
        do_shutdown(chanref chan, unsigned flags, const timeval& now);

        sockstate
        do_state() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const sessionptr& session, void* user, unsigned id);
    };

    typedef smartptr<listener> listenerptr;
}

#endif // AUGASPP_LISTENER_HPP
