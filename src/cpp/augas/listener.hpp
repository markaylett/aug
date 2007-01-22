/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_LISTENER_HPP
#define AUGAS_LISTENER_HPP

#include "augas/file.hpp"

#include "augsyspp.hpp"

namespace augas {

    class listener : public file_base {

        sessptr sess_;
        augas_file file_;
        aug::smartfd sfd_;

        augas_file&
        do_file();

        const augas_file&
        do_file() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const sessptr& sess, augas_id id, void* user,
                 const aug::smartfd& sfd);
    };

    typedef aug::smartptr<listener> listenerptr;
}

#endif // AUGAS_LISTENER_HPP
