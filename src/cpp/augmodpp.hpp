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
#ifndef AUGMODPP_HPP
#define AUGMODPP_HPP

#include "augmod.h"
#include "augses.h"

#include <stdexcept>

#define MOD_WRITELOGCATCH                                   \
    catch (const std::exception& e) {                       \
        mod_writelog(MOD_LOGERROR,                          \
                     "std::exception: %s", e.what());       \
    } catch (...) {                                         \
        mod_writelog(MOD_LOGERROR, "unknown exception");    \
    } do { } while (0)

namespace mod {

    inline void
    writelog(unsigned level, const char* format, ...) AUG_NOTHROW
    {
        va_list args;
        va_start(args, format);
        mod_vwritelog(level, format, args);
        va_end(args);
    }

    inline void
    vwritelog(unsigned level, const char* format, va_list args) AUG_NOTHROW
    {
        mod_vwritelog(level, format, args);
    }

    inline void
    reconfall()
    {
        if (mod_reconfall() < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline void
    stopall()
    {
        if (mod_stopall() < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline void
    post(const char* to, const char* type, mod_id id,
         aug::objectref ob = null)
    {
        if (mod_post(to, type, id, ob.get()) < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline void
    dispatch(const char* to, const char* type, mod_id id,
             aug::objectref ob = null)
    {
        if (mod_dispatch(to, type, id, ob.get()) < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline const char*
    getenv(const char* name, const char* def = 0)
    {
        return mod_getenv(name, def);
    }

    inline void
    shutdown(mod_id sid, unsigned flags)
    {
        if (mod_shutdown(sid, flags) < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline void
    shutdown(const mod_handle& sock, unsigned flags)
    {
        shutdown(sock.id_, flags);
    }

    inline mod_id
    tcpconnect(const char* host, const char* port, const char* sslctx = 0,
               aug::objectref ob = null)
    {
        mod_rint result(mod_tcpconnect(host, port, sslctx, ob.get()));
        if (result < 0)
            throw std::runtime_error(mod_geterror());
        return static_cast<mod_id>(result);
    }

    inline mod_id
    tcplisten(const char* host, const char* port, const char* sslctx = 0,
              aug::objectref ob = null)
    {
        mod_rint result(mod_tcplisten(host, port, sslctx, ob.get()));
        if (result < 0)
            throw std::runtime_error(mod_geterror());
        return static_cast<mod_id>(result);
    }

    inline void
    send(mod_id cid, const void* buf, size_t size)
    {
        if (mod_send(cid, buf, size) < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline void
    send(const mod_handle& conn, const void* buf, size_t size)
    {
        send(conn.id_, buf, size);
    }

    inline void
    sendv(mod_id cid, aug_blob_* blob)
    {
        if (mod_sendv(cid, blob) < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline void
    sendv(const mod_handle& conn, aug_blob_* blob)
    {
        sendv(conn.id_, blob);
    }

    inline void
    setrwtimer(mod_id cid, unsigned ms, unsigned flags)
    {
        if (mod_setrwtimer(cid, ms, flags) < 0)
            throw std::runtime_error(mod_geterror());
    }

    inline void
    setrwtimer(const mod_handle& conn, unsigned ms, unsigned flags)
    {
        setrwtimer(conn.id_, ms, flags);
    }

    inline bool
    resetrwtimer(mod_id cid, unsigned ms, unsigned flags)
    {
        switch (mod_resetrwtimer(cid, ms, flags)) {
        case MOD_FAILERROR:
            throw std::runtime_error(mod_geterror());
        case MOD_FAILNONE:
            return false;
        }
        return true;
    }

    inline bool
    retsetrwtimer(const mod_handle& conn, unsigned ms, unsigned flags)
    {
        return resetrwtimer(conn.id_, ms, flags);
    }

    inline bool
    cancelrwtimer(mod_id cid, unsigned flags)
    {
        switch (mod_cancelrwtimer(cid, flags)) {
        case MOD_FAILERROR:
            throw std::runtime_error(mod_geterror());
        case MOD_FAILNONE:
            return false;
        }
        return true;
    }

    inline bool
    cancelrwtimer(const mod_handle& conn, unsigned flags)
    {
        return cancelrwtimer(conn.id_, flags);
    }

    inline mod_id
    settimer(unsigned ms, aug::objectref ob = null)
    {
        mod_rint result(mod_settimer(ms, ob.get()));
        if (result < 0)
            throw std::runtime_error(mod_geterror());
        return static_cast<mod_id>(result);
    }

    inline bool
    resettimer(mod_id tid, unsigned ms)
    {
        switch (mod_resettimer(tid, ms)) {
        case MOD_FAILERROR:
            throw std::runtime_error(mod_geterror());
        case MOD_FAILNONE:
            return false;
        }
        return true;
    }

    inline bool
    resettimer(const mod_handle& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    inline bool
    canceltimer(mod_id tid)
    {
        switch (mod_canceltimer(tid)) {
        case MOD_FAILERROR:
            throw std::runtime_error(mod_geterror());
        case MOD_FAILNONE:
            return false;
        }
        return true;
    }

    inline bool
    canceltimer(const mod_handle& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    inline void
    emit(unsigned short type, const void* buf, size_t size)
    {
        if (mod_emit(type, buf, size) < 0)
            throw std::runtime_error(mod_geterror());
    }

    struct nil {
        static sessionptr
        create(const char* sname)
        {
            return null;
        }
    };

    template <typename headT, typename tailT>
    struct cons {
        static sessionptr
        create(const char* sname)
        {
            sessionptr ptr(headT::create(sname));
            return null == ptr ? tailT::create(sname) : ptr;
        }
    };

    template <typename listT>
    struct basic_factory {
        static sessionptr
        create(const char* sname)
        {
            mod_writelog(MOD_LOGINFO, "creating session: name=[%s]",
                         sname);
            return listT::create(sname);
        }
    };

    template <typename factoryT>
    class basic_module {
    public:
        static mod_bool
        init(const char* name) AUG_NOTHROW
        {
            return MOD_TRUE;
        }
        static void
        term() AUG_NOTHROW
        {
        }
        static mod_session*
        create(const char* name) AUG_NOTHROW
        {
            return retget(factoryT::create(name));
        }
    };

    template <typename T>
    class basic_session : public session_base<T> {
    public:
        ~basic_session() AUG_NOTHROW
        {
            // Deleted from base.
        }
        mod_bool
        start_() AUG_NOTHROW
        {
            try {
                return static_cast<T*>(this)->start();
            } MOD_WRITELOGCATCH;
            return MOD_FALSE;
        }
        void
        stop_() AUG_NOTHROW
        {
            try {
                return static_cast<T*>(this)->stop();
            } MOD_WRITELOGCATCH;
        }
        void
        reconf_() AUG_NOTHROW
        {
            try {
                return static_cast<T*>(this)->reconf();
            } MOD_WRITELOGCATCH;
        }
        void
        event_(const char* from, const char* type, mod_id id,
               aug::objectref ob = null) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->event(from, type, id, ob.get());
            } MOD_WRITELOGCATCH;
        }
        void
        closed_(mod_handle& sock) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->closed(sock);
            } MOD_WRITELOGCATCH;
        }
        void
        teardown_(mod_handle& sock) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->teardown(sock);
            } MOD_WRITELOGCATCH;
        }
        mod_bool
        accepted_(mod_handle& sock, const char* name) AUG_NOTHROW
        {
            try {
                return static_cast<T*>(this)->accepted(sock, name);
            } MOD_WRITELOGCATCH;
            return MOD_FALSE;
        }
        void
        connected_(mod_handle& sock, const char* name) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->connected(sock, name);
            } MOD_WRITELOGCATCH;
        }
        mod_bool
        auth_(mod_handle& sock, const char* subject,
              const char* issuer) AUG_NOTHROW
        {
            try {
                return static_cast<T*>(this)->auth(sock, subject, issuer);
            } MOD_WRITELOGCATCH;
            return MOD_FALSE;
        }
        void
        recv_(mod_handle& sock, const void* buf,
              size_t len) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->recv(sock, buf, len);
            } MOD_WRITELOGCATCH;
        }
        void
        mrecv_(const char* node, unsigned sess, unsigned short type,
               const void* buf, size_t len) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->mrecv(node, sess, type, buf, len);
            } MOD_WRITELOGCATCH;
        }
        void
        error_(mod_handle& sock, const char* desc) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->error(sock, desc);
            } MOD_WRITELOGCATCH;
        }
        void
        rdexpire_(mod_handle& sock, unsigned& ms) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->rdexpire(sock, ms);
            } MOD_WRITELOGCATCH;
        }
        void
        wrexpire_(mod_handle& sock, unsigned& ms) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->wrexpire(sock, ms);
            } MOD_WRITELOGCATCH;
        }
        void
        expire_(mod_handle& timer, unsigned& ms) AUG_NOTHROW
        {
            try {
                static_cast<T*>(this)->expire(timer, ms);
            } MOD_WRITELOGCATCH;
        }
    };
}

#endif // AUGMODPP_HPP
