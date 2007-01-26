/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_LISTENER_HPP
#define AUGAS_LISTENER_HPP

#include "augas/sock.hpp"

#include "augsyspp.hpp"

namespace augas {

    class listener : public sock_base {

        sessptr sess_;
        augas_sock sock_;
        aug::smartfd sfd_;

        augas_sock&
        do_sock();

        const augas_sock&
        do_sock() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const sessptr& sess, void* user, const aug::smartfd& sfd);
    };

    typedef aug::smartptr<listener> listenerptr;
}

#endif // AUGAS_LISTENER_HPP
