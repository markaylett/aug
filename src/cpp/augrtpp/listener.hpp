/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_LISTENER_HPP
#define AUGRTPP_LISTENER_HPP

#include "augrtpp/object.hpp"

#include "augsyspp.hpp"

namespace aug {

    class listener : public object_base {

        aug::servptr serv_;
        augas_object sock_;
        aug::smartfd sfd_;

        augas_object&
        do_get();

        const augas_object&
        do_get() const;

        const aug::servptr&
        do_serv() const;

        aug::smartfd
        do_sfd() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const aug::servptr& serv, void* user,
                 const aug::smartfd& sfd);
    };

    typedef aug::smartptr<listener> listenerptr;
}

#endif // AUGRTPP_LISTENER_HPP
