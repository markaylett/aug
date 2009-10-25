/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGASPP_ENGINE_HPP
#define AUGASPP_ENGINE_HPP

#include "augaspp/session.hpp"

#include "augsyspp/types.hpp"

#include "augutil/event.h"
#include "augutil/timer.h"

#include "augext/blob.h"

#include "augmod.h"

#include <string>

namespace aug {

    class AUGASPP_API sslctx;

    namespace detail {
        struct engineimpl;
    }

    class AUGASPP_API enginecb_base {
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

    class AUGASPP_API engine : public mpool_ops {

        detail::engineimpl* const impl_;

        engine(const engine& rhs);

        engine&
        operator =(const engine& rhs);

    public:
        ~engine() AUG_NOTHROW;

        engine(aug_events_t events, aug_timers_t timers, enginecb_base& cb);

        void
        clear();

        void
        insert(unsigned id, const std::string& name);

        void
        insert(const std::string& name, const sessionptr& session,
               const char* topics);

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
        post(const char* sname, const char* to, const char* type, mod_id id,
             objectref ob);

        // Thread-unsafe host interface.

        void
        dispatch(const char* sname, const char* to, const char* type,
                 mod_id id, objectref ob);

        void
        shutdown(mod_id cid, unsigned flags);

        mod_id
        tcpconnect(const char* sname, const char* host, const char* port,
                   sslctx* ctx, objectref ob);

        mod_id
        tcplisten(const char* sname, const char* host, const char* port,
                  sslctx* ctx, objectref ob);

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
        settimer(const char* sname, unsigned ms, objectref ob);

        bool
        resettimer(mod_id tid, unsigned ms);

        bool
        canceltimer(mod_id tid);

        void
        emit(const char* node, const char* type, const void* buf, size_t len);

        bool
        stopping() const;
    };
}

#endif // AUGASPP_ENGINE_HPP
