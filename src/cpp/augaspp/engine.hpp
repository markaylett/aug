/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_ENGINE_HPP
#define AUGRTPP_ENGINE_HPP

#include "augaspp/session.hpp"

#include "augnetpp/nbfile.hpp"

#include "maud.h"

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
             objectref ref);

        // Thread-unsafe host interface.

        void
        dispatch(const char* sname, const char* to, const char* type,
                 objectref ref);

        void
        shutdown(maud_id cid, unsigned flags);

        maud_id
        tcpconnect(const char* sname, const char* host, const char* port,
                   void* user);

        maud_id
        tcplisten(const char* sname, const char* host, const char* port,
                  void* user);

        void
        send(maud_id cid, const void* buf, size_t len);

        void
        sendv(maud_id cid, blobref ref);

        void
        setrwtimer(maud_id cid, unsigned ms, unsigned flags);

        bool
        resetrwtimer(maud_id cid, unsigned ms, unsigned flags);

        bool
        cancelrwtimer(maud_id cid, unsigned flags);

        maud_id
        settimer(const char* sname, unsigned ms, objectref ref);

        bool
        resettimer(maud_id tid, unsigned ms);

        bool
        canceltimer(maud_id tid);

        void
        setsslclient(maud_id cid, sslctx& ctx);

        void
        setsslserver(maud_id cid, sslctx& ctx);

        bool
        stopping() const;
    };
}

#endif // AUGRTPP_ENGINE_HPP
