/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_ENGINE_HPP
#define AUGRTPP_ENGINE_HPP

#include "augaspp/session.hpp"

#include "augsyspp/types.hpp"

#include "augext/blob.h"

#include "augmod.h"

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

        engine(mdref eventrd, mdref eventwr, timers& timers,
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
             aug::objectref ob);

        // Thread-unsafe host interface.

        void
        dispatch(const char* sname, const char* to, const char* type,
                 aug::objectref ob);

        void
        shutdown(mod_id cid, unsigned flags);

        mod_id
        tcpconnect(const char* sname, const char* host, const char* port,
                   sslctx& ctx, void* user);

        mod_id
        tcplisten(const char* sname, const char* host, const char* port,
                  sslctx& ctx, void* user);

        void
        send(mod_id cid, const void* buf, size_t len);

        void
        sendv(mod_id cid, blobref ref);

        void
        setrwtimer(mod_id cid, unsigned ms, unsigned flags);

        bool
        resetrwtimer(mod_id cid, unsigned ms, unsigned flags);

        bool
        cancelrwtimer(mod_id cid, unsigned flags);

        mod_id
        settimer(const char* sname, unsigned ms, aug::objectref ob);

        bool
        resettimer(mod_id tid, unsigned ms);

        bool
        canceltimer(mod_id tid);

        bool
        stopping() const;
    };
}

#endif // AUGRTPP_ENGINE_HPP
