/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_ENGINE_HPP
#define AUGRTPP_ENGINE_HPP

#include "augaspp/session.hpp"

#include "augnetpp/nbfile.hpp"

#include "aum.h"

namespace aug {

    class AUGRTPP_API sslctx;
    class timers;

    namespace detail {
        struct engineimpl;
    }

    class AUGRTPP_API enginecb_base {
        virtual void
        do_reconf() = 0;

    public:
        virtual
        ~enginecb_base() AUG_NOTHROW;

        void
        reconf()
        {
            do_reconf();
        }
    };

    class AUGRTPP_API engine {

        detail::engineimpl* const impl_;

        engine(const engine& rhs);

        engine&
        operator =(const engine& rhs);

    public:
        ~engine() AUG_NOTHROW;

        engine(const smartfd& eventrd, const smartfd& eventwr, timers& timers,
               enginecb_base& cb);

        void
        clear();

        void
        insert(const std::string& name, const sessionptr& session,
               const char* groups);

        void
        cancelinactive();

        void
        run(bool stoponerr);

        // Thread-safe host interface.

        void
        reconfall();

        void
        stopall();

        void
        post(const char* sname, const char* to, const char* type,
             aub::objectref ob);

        // Thread-unsafe host interface.

        void
        dispatch(const char* sname, const char* to, const char* type,
                 aub::objectref ob);

        void
        shutdown(aum_id cid, unsigned flags);

        aum_id
        tcpconnect(const char* sname, const char* host, const char* port,
                   void* user);

        aum_id
        tcplisten(const char* sname, const char* host, const char* port,
                  void* user);

        void
        send(aum_id cid, const void* buf, size_t len);

        void
        sendv(aum_id cid, blobref ref);

        void
        setrwtimer(aum_id cid, unsigned ms, unsigned flags);

        bool
        resetrwtimer(aum_id cid, unsigned ms, unsigned flags);

        bool
        cancelrwtimer(aum_id cid, unsigned flags);

        aum_id
        settimer(const char* sname, unsigned ms, aub::objectref ob);

        bool
        resettimer(aum_id tid, unsigned ms);

        bool
        canceltimer(aum_id tid);

        void
        setsslclient(aum_id cid, sslctx& ctx);

        void
        setsslserver(aum_id cid, sslctx& ctx);

        bool
        stopping() const;
    };
}

#endif // AUGRTPP_ENGINE_HPP
