/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_HPP
#define AUGASPP_HPP

#include "augas.h"

#include <stdexcept>

#if !defined(AUGAS_NOTHROW)
# define AUGAS_NOTHROW throw()
#endif // !AUGAS_NOTHROW

#define AUGAS_WRITELOGCATCH                                     \
    catch (const std::exception& e) {                           \
        augas_writelog(AUGAS_LOGERROR,                          \
                       "std::exception: %s", e.what());         \
    } catch (...) {                                             \
        augas_writelog(AUGAS_LOGERROR, "unknown exception");    \
    } do { } while (0)

namespace augas {

    class event {
        const augas_event& ref_;
    public:
        explicit
        event(const augas_event& ref)
            : ref_(ref)
        {
        }
        const char*
        type() const
        {
            return ref_.type_;
        }
        void*
        user() const
        {
            return ref_.user_;
        }
        size_t
        size() const
        {
            return ref_.size_;
        }
    };

    class object {
        const augas_object& ref_;
    public:
        explicit
        object(const augas_object& ref)
            : ref_(ref)
        {
        }
        void
        setuser(void* user)
        {
            const_cast<augas_object&>(ref_).user_ = user;
        }
        const char*
        sname() const
        {
            return ref_.serv_->name_;
        }
        augas_id
        id() const
        {
            return ref_.id_;
        }
        void*
        user() const
        {
            return ref_.user_;
        }
    };

    class serv_base {
        typedef augas::event event_type;
        virtual bool
        do_start(const char* sname)
        {
            return true;
        }
        virtual bool
        do_reconf(const char* sname)
        {
            return true;
        }
        virtual bool
        do_event(const char* sname, const char* from, const event_type& event)
        {
            return true;
        }
        virtual void
        do_closed(const object& sock)
        {
        }
        virtual bool
        do_teardown(const object& sock)
        {
            return true;
        }
        virtual bool
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            return true;
        }
        virtual bool
        do_connected(object& sock, const char* addr, unsigned short port)
        {
            return true;
        }
        virtual bool
        do_data(const object& sock, const char* buf, size_t size)
        {
            return true;
        }
        virtual bool
        do_rdexpire(const object& sock, unsigned& ms)
        {
            return true;
        }
        virtual bool
        do_wrexpire(const object& sock, unsigned& ms)
        {
            return true;
        }
        virtual bool
        do_expire(const object& timer, unsigned& ms)
        {
            return true;
        }
    public:
        virtual
        ~serv_base() AUGAS_NOTHROW
        {
        }
        bool
        start(const char* sname)
        {
            return do_start(sname);
        }
        bool
        reconf(const char* sname)
        {
            return do_reconf(sname);
        }
        bool
        event(const char* sname, const char* from, const event_type& event)
        {
            return do_event(sname, from, event);
        }
        void
        closed(const object& sock)
        {
            do_closed(sock);
        }
        bool
        teardown(const object& sock)
        {
            return do_teardown(sock);
        }
        bool
        accept(object& sock, const char* addr, unsigned short port)
        {
            return do_accept(sock, addr, port);
        }
        bool
        connected(object& sock, const char* addr, unsigned short port)
        {
            return do_connected(sock, addr, port);
        }
        bool
        data(const object& sock, const char* buf, size_t size)
        {
            return do_data(sock, buf, size);
        }
        bool
        rdexpire(const object& sock, unsigned& ms)
        {
            return do_rdexpire(sock, ms);
        }
        bool
        wrexpire(const object& sock, unsigned& ms)
        {
            return do_wrexpire(sock, ms);
        }
        bool
        expire(const object& timer, unsigned& ms)
        {
            return do_expire(timer, ms);
        }
    };

    template <typename T>
    class basic_module {
        static T* impl_;
        static int
        result(bool x)
        {
            return x ? AUGAS_OK : AUGAS_ERROR;
        }
        static void
        stop(const augas_serv* serv) AUGAS_NOTHROW
        {
            delete static_cast<serv_base*>(serv->user_);
        }
        static int
        start(augas_serv* serv) AUGAS_NOTHROW
        {
            try {
                serv->user_ = impl_->create(serv->name_);
                return result(static_cast<serv_base*>(serv->user_)
                              ->start(serv->name_));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        reconf(const augas_serv* serv) AUGAS_NOTHROW
        {
            try {
                return result(static_cast<serv_base*>(serv->user_)
                              ->reconf(serv->name_));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        event(const augas_serv* serv, const char* from,
              const augas_event* event) AUGAS_NOTHROW
        {
            try {
                return result(static_cast<serv_base*>(serv->user_)
                              ->event(serv->name_, from,
                                      augas::event(*event)));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static void
        closed(const augas_object* sock) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(sock->serv_->user_)
                    ->closed(object(*sock));
            } AUGAS_WRITELOGCATCH;
        }
        static int
        teardown(const augas_object* sock) AUGAS_NOTHROW
        {
            try {
                return result(static_cast<serv_base*>(sock->serv_->user_)
                              ->teardown(object(*sock)));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        accept(augas_object* sock, const char* addr,
               unsigned short port) AUGAS_NOTHROW
        {
            try {
                object o(*sock);
                return result(static_cast<serv_base*>(sock->serv_->user_)
                              ->accept(o, addr, port));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        connected(augas_object* sock, const char* addr,
                  unsigned short port) AUGAS_NOTHROW
        {
            try {
                object o(*sock);
                return result(static_cast<serv_base*>(sock->serv_->user_)
                              ->connected(o, addr, port));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        data(const augas_object* sock, const char* buf,
             size_t size) AUGAS_NOTHROW
        {
            try {
                return result(static_cast<serv_base*>(sock->serv_->user_)
                              ->data(object(*sock), buf, size));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        rdexpire(const augas_object* sock, unsigned* ms) AUGAS_NOTHROW
        {
            try {
                return result(static_cast<serv_base*>(sock->serv_->user_)
                              ->rdexpire(object(*sock), *ms));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        wrexpire(const augas_object* sock, unsigned* ms) AUGAS_NOTHROW
        {
            try {
                return result(static_cast<serv_base*>(sock->serv_->user_)
                              ->wrexpire(object(*sock), *ms));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
        static int
        expire(const augas_object* timer, unsigned* ms) AUGAS_NOTHROW
        {
            try {
                return result(static_cast<serv_base*>(timer->serv_->user_)
                              ->expire(object(*timer), *ms));
            } AUGAS_WRITELOGCATCH;
            return AUGAS_ERROR;
        }
    public:
        static const struct augas_module*
        init(const char* name) AUGAS_NOTHROW
        {
            static const augas_module module_ = {
                stop,
                start,
                reconf,
                event,
                closed,
                teardown,
                accept,
                connected,
                data,
                rdexpire,
                wrexpire,
                expire
            };
            try {
                impl_ = new T(name);
                return &module_;
            } AUGAS_WRITELOGCATCH;
            return 0; // Error.
        }
        static void
        term() AUGAS_NOTHROW
        {
            delete impl_;
            impl_ = 0;
        }
    };

    template <typename T>
    T* basic_module<T>::impl_ = 0;

    typedef std::runtime_error error;

    inline void
    writelog(int level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        augas_vwritelog(level, format, args);
        va_end(args);
    }

    inline void
    vwritelog(int level, const char* format, va_list args)
    {
        augas_vwritelog(level, format, args);
    }

    inline void
    reconf()
    {
        augas_reconf();
    }

    //...

    inline void
    shutdown(augas_id sid)
    {
        if (-1 == augas_shutdown(sid))
            throw error(augas_error());
    }
}

#endif // AUGASPP_HPP
