/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_LISTENER_HPP
#define DAUG_LISTENER_HPP

#include "daug/object.hpp"

#include "augsyspp.hpp"

namespace augas {

    class listener : public object_base {

        servptr serv_;
        augas_object sock_;
        aug::smartfd sfd_;

        augas_object&
        do_object();

        const augas_object&
        do_object() const;

        const servptr&
        do_serv() const;

        aug::smartfd
        do_sfd() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const servptr& serv, void* user, const aug::smartfd& sfd);
    };

    typedef aug::smartptr<listener> listenerptr;
}

#endif // DAUG_LISTENER_HPP
