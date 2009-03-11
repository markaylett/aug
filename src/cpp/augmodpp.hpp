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

#include <stdexcept>

#if !defined(NDEBUG)
# define MOD_NOTHROW throw()
#else /* NDEBUG */
# define MOD_NOTHROW
#endif /* NDEBUG */

#define MOD_WRITELOGCATCH                                   \
    catch (const std::exception& e) {                       \
        mod_writelog(MOD_LOGERROR,                          \
                     "std::exception: %s", e.what());       \
    } catch (...) {                                         \
        mod_writelog(MOD_LOGERROR, "unknown exception");    \
    } do { } while (0)

namespace aug {
namespace mod {

    typedef std::runtime_error error;

    inline void
    writelog(int level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        mod_vwritelog(level, format, args);
        va_end(args);
    }

    inline void
    vwritelog(int level, const char* format, va_list args)
    {
        mod_vwritelog(level, format, args);
    }

    inline void
    reconfall()
    {
        if (mod_reconfall() < 0)
            throw error(mod_error());
    }

    inline void
    stopall()
    {
        if (mod_stopall() < 0)
            throw error(mod_error());
    }

    inline void
    post(mod_id id, const char* to, const char* type, aug_object_* ob)
    {
        if (mod_post(id, to, type, ob) < 0)
            throw error(mod_error());
    }

    inline void
    dispatch(mod_id id, const char* to, const char* type, aug_object_* ob)
    {
        if (mod_dispatch(id, to, type, ob) < 0)
            throw error(mod_error());
    }

    inline const char*
    getenv(const char* name, const char* def = 0)
    {
        return mod_getenv(name, def);
    }

    inline const mod_session*
    getsession()
    {
        return mod_getsession();
    }

    inline void
    shutdown(mod_id sid, unsigned flags)
    {
        if (mod_shutdown(sid, flags) < 0)
            throw error(mod_error());
    }

    inline void
    shutdown(const mod_handle& sock, unsigned flags)
    {
        shutdown(sock.id_, flags);
    }

    inline mod_id
    tcpconnect(const char* host, const char* port, const char* sslctx = 0,
               void* user = 0)
    {
        int ret(mod_tcpconnect(host, port, sslctx, user));
        if (ret < 0)
            throw error(mod_error());
        return static_cast<mod_id>(ret);
    }

    inline mod_id
    tcplisten(const char* host, const char* port, const char* sslctx = 0,
              void* user = 0)
    {
        int ret(mod_tcplisten(host, port, sslctx, user));
        if (ret < 0)
            throw error(mod_error());
        return static_cast<mod_id>(ret);
    }

    inline void
    send(mod_id cid, const void* buf, size_t size)
    {
        if (mod_send(cid, buf, size) < 0)
            throw error(mod_error());
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
            throw error(mod_error());
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
            throw error(mod_error());
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
            throw error(mod_error());
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
            throw error(mod_error());
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
    settimer(unsigned ms, aug_object_* ob)
    {
        int ret(mod_settimer(ms, ob));
        if (ret < 0)
            throw error(mod_error());
        return static_cast<mod_id>(ret);
    }

    inline bool
    resettimer(mod_id tid, unsigned ms)
    {
        switch (mod_resettimer(tid, ms)) {
        case MOD_FAILERROR:
            throw error(mod_error());
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
            throw error(mod_error());
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

    class handle {
        const mod_handle& handle_;
    public:
        explicit
        handle(const mod_handle& handle)
            : handle_(handle)
        {
        }
        void
        setuser(void* user)
        {
            const_cast<mod_handle&>(handle_).user_ = user;
        }
        mod_id
        id() const
        {
            return handle_.id_;
        }
        void*
        user() const
        {
            return handle_.user_;
        }
        template <typename T>
        T*
        user() const
        {
            return static_cast<T*>(handle_.user_);
        }
        operator const mod_handle&() const
        {
            return handle_;
        }
    };

    class session_base {
        virtual bool
        do_start(const char* sname) = 0;

        virtual void
        do_reconf() = 0;

        virtual void
        do_event(mod_id id, const char* from, const char* type,
                 aug_object_* ob) = 0;

        virtual void
        do_closed(const handle& sock) = 0;

        virtual void
        do_teardown(const handle& sock) = 0;

        virtual bool
        do_accepted(handle& sock, const char* name) = 0;

        virtual void
        do_connected(handle& sock, const char* name) = 0;

        virtual bool
        do_auth(const handle& sock, const char* subject,
                const char* issuer) = 0;

        virtual void
        do_recv(const handle& sock, const void* buf, size_t size) = 0;

        virtual void
        do_error(const handle& sock, const char* desc) = 0;

        virtual void
        do_rdexpire(const handle& sock, unsigned& ms) = 0;

        virtual void
        do_wrexpire(const handle& sock, unsigned& ms) = 0;

        virtual void
        do_expire(const handle& timer, unsigned& ms) = 0;

    public:
        virtual
        ~session_base() MOD_NOTHROW
        {
        }
        bool
        start(const char* sname)
        {
            return do_start(sname);
        }
        void
        reconf()
        {
            do_reconf();
        }
        void
        event(mod_id id, const char* from, const char* type, aug_object_* ob)
        {
            do_event(id, from, type, ob);
        }
        void
        closed(const handle& sock)
        {
            do_closed(sock);
        }
        void
        teardown(const handle& sock)
        {
            do_teardown(sock);
        }
        bool
        accepted(handle& sock, const char* name)
        {
            return do_accepted(sock, name);
        }
        void
        connected(handle& sock, const char* name)
        {
            do_connected(sock, name);
        }
        bool
        auth(const handle& sock, const char* subject, const char* issuer)
        {
            return do_auth(sock, subject, issuer);
        }
        void
        recv(const handle& sock, const void* buf, size_t size)
        {
            do_recv(sock, buf, size);
        }
        void
        error(const handle& sock, const char* desc)
        {
            do_error(sock, desc);
        }
        void
        rdexpire(const handle& sock, unsigned& ms)
        {
            do_rdexpire(sock, ms);
        }
        void
        wrexpire(const handle& sock, unsigned& ms)
        {
            do_wrexpire(sock, ms);
        }
        void
        expire(const handle& timer, unsigned& ms)
        {
            do_expire(timer, ms);
        }
    };

    class basic_session : public session_base {
        void
        do_reconf()
        {
            mod_writelog(MOD_LOGWARN, "do_reconf() not implemented");
        }
        void
        do_event(mod_id id, const char* from, const char* type,
                 aug_object_* ob)
        {
            mod_writelog(MOD_LOGWARN, "do_event() not implemented");
        }
        void
        do_closed(const handle& sock)
        {
            mod_writelog(MOD_LOGWARN, "do_closed() not implemented");
        }
        void
        do_teardown(const handle& sock)
        {
            mod_writelog(MOD_LOGINFO, "teardown defaulting to shutdown");
            shutdown(sock, 0);
        }
        bool
        do_accepted(handle& sock, const char* name)
        {
            mod_writelog(MOD_LOGWARN, "do_accepted() not implemented");
            return true;
        }
        void
        do_connected(handle& sock, const char* name)
        {
            mod_writelog(MOD_LOGWARN, "do_connected() not implemented");
        }
        bool
        do_auth(const handle& sock, const char* subject, const char* issuer)
        {
            mod_writelog(MOD_LOGWARN, "do_auth() not implemented");
            return true;
        }
        void
        do_recv(const handle& sock, const void* buf, size_t size)
        {
            mod_writelog(MOD_LOGWARN, "do_recv() not implemented");
        }
        void
        do_error(const handle& sock, const char* desc)
        {
            mod_writelog(MOD_LOGERROR, "error: %s", desc);
        }
        void
        do_rdexpire(const handle& sock, unsigned& ms)
        {
            mod_writelog(MOD_LOGWARN, "do_rdexpire() not implemented");
        }
        void
        do_wrexpire(const handle& sock, unsigned& ms)
        {
            mod_writelog(MOD_LOGWARN, "do_wrexpire() not implemented");
        }
        void
        do_expire(const handle& timer, unsigned& ms)
        {
            mod_writelog(MOD_LOGWARN, "do_expire() not implemented");
        }

    public:
        virtual
        ~basic_session() MOD_NOTHROW
        {
        }
    };

    struct nil {
        static session_base*
        create(const char* sname)
        {
            return 0;
        }
    };

    template <typename headT, typename tailT>
    struct cons {
        static session_base*
        create(const char* sname)
        {
            session_base* p = headT::create(sname);
            return p ? p : tailT::create(sname);
        }
    };

    template <typename listT>
    struct basic_factory {
        explicit
        basic_factory(const char* module)
        {
            mod_writelog(MOD_LOGINFO, "creating factory: module=[%s]",
                         module);
        }
        session_base*
        create(const char* sname)
        {
            mod_writelog(MOD_LOGINFO, "creating session: name=[%s]",
                         sname);
            return listT::create(sname);
        }
    };

    template <typename T>
    class basic_module {
        static T* factory_;
        static session_base*
        getbase()
        {
            return static_cast<session_base*>(getsession()->user_);
        }
        static mod_bool
        tobool(bool x)
        {
            return x ? MOD_TRUE : MOD_FALSE;
        }
        static void
        stop() MOD_NOTHROW
        {
            delete getbase();
        }
        static mod_bool
        start(mod_session* session) MOD_NOTHROW
        {
            try {
                session->user_ = factory_->create(session->name_);
                return tobool(getbase()->start(session->name_));
            } MOD_WRITELOGCATCH;
            return MOD_FALSE;
        }
        static void
        reconf() MOD_NOTHROW
        {
            try {
                getbase()->reconf();
            } MOD_WRITELOGCATCH;
        }
        static void
        event(mod_id id, const char* from, const char* type,
              aug_object_* ob) MOD_NOTHROW
        {
            try {
                getbase()->event(id, from, type, ob);
            } MOD_WRITELOGCATCH;
        }
        static void
        closed(const mod_handle* sock) MOD_NOTHROW
        {
            try {
                getbase()->closed(handle(*sock));
            } MOD_WRITELOGCATCH;
        }
        static void
        teardown(const mod_handle* sock) MOD_NOTHROW
        {
            try {
                getbase()->teardown(handle(*sock));
            } MOD_WRITELOGCATCH;
        }
        static mod_bool
        accepted(mod_handle* sock, const char* name) MOD_NOTHROW
        {
            try {
                handle h(*sock);
                return tobool(getbase()->accepted(h, name));
            } MOD_WRITELOGCATCH;
            return MOD_FALSE;
        }
        static void
        connected(mod_handle* sock, const char* name) MOD_NOTHROW
        {
            try {
                handle h(*sock);
                getbase()->connected(h, name);
            } MOD_WRITELOGCATCH;
        }
        static mod_bool
        auth(const mod_handle* sock, const char* subject, const char* issuer)
        {
            try {
                handle h(*sock);
                return tobool(getbase()->auth(h, subject, issuer));
            } MOD_WRITELOGCATCH;
            return MOD_FALSE;
        }
        static void
        recv(const mod_handle* sock, const void* buf,
             size_t size) MOD_NOTHROW
        {
            try {
                getbase()->recv(handle(*sock), buf, size);
            } MOD_WRITELOGCATCH;
        }
        static void
        error(const mod_handle* sock, const char* desc) MOD_NOTHROW
        {
            try {
                getbase()->error(handle(*sock), desc);
            } MOD_WRITELOGCATCH;
        }
        static void
        rdexpire(const mod_handle* sock, unsigned* ms) MOD_NOTHROW
        {
            try {
                getbase()->rdexpire(handle(*sock), *ms);
            } MOD_WRITELOGCATCH;
        }
        static void
        wrexpire(const mod_handle* sock, unsigned* ms) MOD_NOTHROW
        {
            try {
                getbase()->wrexpire(handle(*sock), *ms);
            } MOD_WRITELOGCATCH;
        }
        static void
        expire(const mod_handle* timer, unsigned* ms) MOD_NOTHROW
        {
            try {
                getbase()->expire(handle(*timer), *ms);
            } MOD_WRITELOGCATCH;
        }

    public:
        static const mod_module*
        init(const char* name) MOD_NOTHROW
        {
            static const mod_module local = {
                stop,
                start,
                reconf,
                event,
                closed,
                teardown,
                accepted,
                connected,
                auth,
                recv,
                error,
                rdexpire,
                wrexpire,
                expire
            };
            try {
                factory_ = new T(name);
                return &local;
            } MOD_WRITELOGCATCH;
            return 0; // Error.
        }
        static void
        term() MOD_NOTHROW
        {
            delete factory_;
            factory_ = 0;
        }
    };

    template <typename T>
    T* basic_module<T>::factory_ = 0;
}}

#endif // AUGMODPP_HPP
