/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_ENGINE_HPP
#define AUGRTPP_ENGINE_HPP

#include "augrtpp/serv.hpp"

#include "augnetpp/nbfile.hpp"

#include "augas.h"

namespace aug {

    class sslctx;

    namespace detail {
        struct engineimpl;
    }

    class enginecb_base {
        virtual void
        do_reconf() = 0;

        virtual void
        do_reopen() = 0;

    public:
        virtual
        ~enginecb_base() AUG_NOTHROW;

        void
        reconf()
        {
            do_reconf();
        }
        void
        reopen()
        {
            do_reopen();
        }
    };

    class engine {

        detail::engineimpl* const impl_;

        engine(const engine& rhs);

        engine&
        operator =(const engine& rhs);

    public:
        ~engine() AUG_NOTHROW;

        engine(fdref rdfd, fdref wrfd, enginecb_base& cb);

        void
        clear();

        void
        post(const char* sname, const char* to, const char* type,
             const aug_var* var);

        void
        dispatch(const char* sname, const char* to, const char* type,
                 const void* user, size_t size);

        void
        shutdown(augas_id cid);

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
        sendv(augas_id cid, const aug_var& var);

        void
        setrwtimer(augas_id cid, unsigned ms, unsigned flags);

        bool
        resetrwtimer(augas_id cid, unsigned ms, unsigned flags);

        bool
        cancelrwtimer(augas_id cid, unsigned flags);

        augas_id
        settimer(const char* sname, unsigned ms, const aug_var* var);

        bool
        resettimer(augas_id tid, unsigned ms);

        bool
        canceltimer(augas_id tid);

        void
        setsslclient(augas_id cid, sslctx& ctx);

        void
        setsslserver(augas_id cid, sslctx& ctx);

        void
        insert(const std::string& name, const servptr& serv,
               const char* groups);

        void
        cancelinactive();

        void
        run(bool daemon);

        bool
        stopping() const;
    };
}

#endif // AUGRTPP_ENGINE_HPP
