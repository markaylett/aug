/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_ENGINE_HPP
#define AUGRTPP_ENGINE_HPP

#include "augrtpp/config.hpp"

#include "augnetpp/nbfile.hpp"

#include "augas.h"

namespace aug {

    namespace detail {
        struct engineimpl;
    }

    class engine {

        detail::engineimpl* const impl_;

        engine(const engine& rhs);

        engine&
        operator =(const engine& rhs);

    public:
        ~engine() AUG_NOTHROW;

        engine(fdref eventfd, aug_nbfilecb_t cb, const aug_var& var);

        void
        dispatch(const char* sname, const char* to, const char* type,
                 const void* user, size_t size);

        void
        shutdown_(augas_id cid);

        void
        teardown();

        augas_id
        tcpconnect(const char* sname, const char* host, const char* port,
                   void* user);

        augas_id
        tcplisten(const char* sname, const char* host, const char* port,
                  void* user);

        void
        send(augas_id cid, const void* buf, size_t len);

        void
        sendv(augas_id cid, const augas_var& var);

        void
        setrwtimer(augas_id cid, unsigned ms, unsigned flags);

        bool
        resetrwtimer(augas_id cid, unsigned ms, unsigned flags);

        bool
        cancelrwtimer(augas_id cid, unsigned flags);

        augas_id
        settimer(const char* sname, unsigned ms, const augas_var& var);

        bool
        resettimer(augas_id tid, unsigned ms);

        bool
        canceltimer(augas_id tid);

        bool
        stopping() const;
    };
}

#endif // AUGRTPP_ENGINE_HPP
