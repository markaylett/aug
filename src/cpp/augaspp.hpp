/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_HPP
#define AUGASPP_HPP

#include "augas.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <functional>

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
        augas_event event_;
    public:
        event()
        {
            memset(&event_, 0, sizeof(event_));
        }
        explicit
        event(const augas_event& event)
            : event_(event)
        {
        }
        explicit
        event(const char* type)
        {
            settype(type);
            event_.user_ = 0;
            event_.size_ = 0;
        }
        template <typename T>
        event(const char* type, T* user, size_t size = 0)
        {
            settype(type);
            setuser(user, size);
        }
        void
        clear()
        {
            event_.type_[0] ='\0';
            event_.user_ = 0;
            event_.size_ = 0;
        }
        void
        settype(const char* type)
        {
            strncpy(event_.type_, type, sizeof(event_.type_));
            event_.type_[AUGAS_MAXNAME] ='\0';
        }
        template <typename T>
        void
        setuser(T* user, size_t size = 0)
        {
            if (user) {
                event_.user_ = static_cast<void*>(user);
                event_.size_ = size;
            } else {
                event_.user_ = 0;
                event_.size_ = 0;
            }
        }
        const char*
        type() const
        {
            return event_.type_;
        }
        template <typename T>
        T*
        user() const
        {
            return static_cast<T>(event_.user_);
        }
        size_t
        size() const
        {
            return event_.size_;
        }
        operator const augas_event&() const
        {
            return event_;
        }
    };

    class object {
        const augas_object& object_;
    public:
        explicit
        object(const augas_object& object)
            : object_(object)
        {
        }
        template <typename T>
        void
        setuser(T* user)
        {
            const_cast<augas_object&>(object_).user_
                = static_cast<void*>(user);
        }
        const char*
        sname() const
        {
            return object_.serv_->name_;
        }
        augas_id
        id() const
        {
            return object_.id_;
        }
        template <typename T>
        T*
        user() const
        {
            return static_cast<T*>(object_.user_);
        }
        operator const augas_object&() const
        {
            return object_;
        }
    };

    class serv_base {
        typedef augas::event event_type;
        virtual bool
        do_start(const char* sname) = 0;

        virtual void
        do_reconf(const char* sname) = 0;

        virtual void
        do_event(const char* sname, const char* from,
                 const event_type& event) = 0;

        virtual void
        do_closed(const object& sock) = 0;

        virtual void
        do_teardown(const object& sock) = 0;

        virtual bool
        do_accept(object& sock, const char* addr, unsigned short port) = 0;

        virtual void
        do_connected(object& sock, const char* addr, unsigned short port) = 0;

        virtual void
        do_data(const object& sock, const char* buf, size_t size) = 0;

        virtual void
        do_rdexpire(const object& sock, unsigned& ms) = 0;

        virtual void
        do_wrexpire(const object& sock, unsigned& ms) = 0;

        virtual void
        do_expire(const object& timer, unsigned& ms) = 0;

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
        void
        reconf(const char* sname)
        {
            do_reconf(sname);
        }
        void
        event(const char* sname, const char* from, const event_type& event)
        {
            do_event(sname, from, event);
        }
        void
        closed(const object& sock)
        {
            do_closed(sock);
        }
        void
        teardown(const object& sock)
        {
            do_teardown(sock);
        }
        bool
        accept(object& sock, const char* addr, unsigned short port)
        {
            return do_accept(sock, addr, port);
        }
        void
        connected(object& sock, const char* addr, unsigned short port)
        {
            do_connected(sock, addr, port);
        }
        void
        data(const object& sock, const char* buf, size_t size)
        {
            do_data(sock, buf, size);
        }
        void
        rdexpire(const object& sock, unsigned& ms)
        {
            do_rdexpire(sock, ms);
        }
        void
        wrexpire(const object& sock, unsigned& ms)
        {
            do_wrexpire(sock, ms);
        }
        void
        expire(const object& timer, unsigned& ms)
        {
            do_expire(timer, ms);
        }
    };

    class basic_serv : public serv_base {
        typedef augas::event event_type;
        void
        do_reconf(const char* sname)
        {
        }
        void
        do_event(const char* sname, const char* from, const event_type& event)
        {
        }
        void
        do_closed(const object& sock)
        {
        }
        void
        do_teardown(const object& sock)
        {
        }
        bool
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            return true;
        }
        void
        do_connected(object& sock, const char* addr, unsigned short port)
        {
        }
        void
        do_data(const object& sock, const char* buf, size_t size)
        {
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
        }
        void
        do_wrexpire(const object& sock, unsigned& ms)
        {
        }
        void
        do_expire(const object& timer, unsigned& ms)
        {
        }
    public:
        virtual
        ~basic_serv() AUGAS_NOTHROW
        {
        }
    };

    template <typename T>
    struct basic_factory {
        explicit
        basic_factory(const char* sname)
        {
        }
        serv_base*
        create(const char* sname)
        {
            augas_writelog(AUGAS_LOGINFO, "creating session [%s]", sname);
            return new T();
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
        static void
        reconf(const augas_serv* serv) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(serv->user_)
                    ->reconf(serv->name_);
            } AUGAS_WRITELOGCATCH;
        }
        static void
        event(const augas_serv* serv, const char* from,
              const augas_event* event) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(serv->user_)
                    ->event(serv->name_, from, augas::event(*event));
            } AUGAS_WRITELOGCATCH;
        }
        static void
        closed(const augas_object* sock) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(sock->serv_->user_)
                    ->closed(object(*sock));
            } AUGAS_WRITELOGCATCH;
        }
        static void
        teardown(const augas_object* sock) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(sock->serv_->user_)
                    ->teardown(object(*sock));
            } AUGAS_WRITELOGCATCH;
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
        static void
        connected(augas_object* sock, const char* addr,
                  unsigned short port) AUGAS_NOTHROW
        {
            try {
                object o(*sock);
                static_cast<serv_base*>(sock->serv_->user_)
                    ->connected(o, addr, port);
            } AUGAS_WRITELOGCATCH;
        }
        static void
        data(const augas_object* sock, const char* buf,
             size_t size) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(sock->serv_->user_)
                    ->data(object(*sock), buf, size);
            } AUGAS_WRITELOGCATCH;
        }
        static void
        rdexpire(const augas_object* sock, unsigned* ms) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(sock->serv_->user_)
                    ->rdexpire(object(*sock), *ms);
            } AUGAS_WRITELOGCATCH;
        }
        static void
        wrexpire(const augas_object* sock, unsigned* ms) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(sock->serv_->user_)
                    ->wrexpire(object(*sock), *ms);
            } AUGAS_WRITELOGCATCH;
        }
        static void
        expire(const augas_object* timer, unsigned* ms) AUGAS_NOTHROW
        {
            try {
                static_cast<serv_base*>(timer->serv_->user_)
                    ->expire(object(*timer), *ms);
            } AUGAS_WRITELOGCATCH;
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
        if (AUGAS_ERROR == augas_reconf())
            throw error(augas_error());
    }

    inline void
    stopall()
    {
        if (AUGAS_ERROR == augas_stopall())
            throw error(augas_error());
    }

    inline void
    post(const char* sname, const char* to, const augas_event& event,
         void (*destroy)(void*) = 0)
    {
        if (AUGAS_ERROR == augas_post(sname, to, &event, destroy))
            throw error(augas_error());
    }

    inline void
    dispatch(const char* sname, const char* to, const augas_event& event)
    {
        if (AUGAS_ERROR == augas_dispatch(sname, to, &event))
            throw error(augas_error());
    }

    inline const char*
    getenv(const char* name)
    {
        return augas_getenv(name);
    }

    inline void
    shutdown(augas_id sid)
    {
        if (AUGAS_ERROR == augas_shutdown(sid))
            throw error(augas_error());
    }

    inline void
    shutdown(const augas_object& sock)
    {
        shutdown(sock.id_);
    }

    inline augas_id
    tcpconnect(const char* sname, const char* host, const char* port,
               void* user = 0)
    {
        int ret(augas_tcpconnect(sname, host, port, user));
        if (AUGAS_ERROR == ret)
            throw error(augas_error());
        return static_cast<augas_id>(ret);
    }

    inline augas_id
    tcplisten(const char* sname, const char* host, const char* port,
              void* user = 0)
    {
        int ret(augas_tcplisten(sname, host, port, user));
        if (AUGAS_ERROR == ret)
            throw error(augas_error());
        return static_cast<augas_id>(ret);
    }

    inline void
    send(augas_id cid, const char* buf, size_t size)
    {
        if (AUGAS_ERROR == augas_send(cid, buf, size))
            throw error(augas_error());
    }

    inline void
    send(const augas_object& conn, const char* buf, size_t size)
    {
        send(conn.id_, buf, size);
    }

    inline void
    setrwtimer(augas_id cid, unsigned ms, unsigned flags)
    {
        if (AUGAS_ERROR == augas_setrwtimer(cid, ms, flags))
            throw error(augas_error());
    }

    inline void
    setrwtimer(const augas_object& conn, unsigned ms, unsigned flags)
    {
        setrwtimer(conn.id_, ms, flags);
    }

    inline bool
    resetrwtimer(augas_id cid, unsigned ms, unsigned flags)
    {
        switch (augas_resetrwtimer(cid, ms, flags)) {
        case AUGAS_ERROR:
            throw error(augas_error());
        case AUGAS_NONE:
            return false;
        }
        return true;
    }

    inline bool
    retsetrwtimer(const augas_object& conn, unsigned ms, unsigned flags)
    {
        return resetrwtimer(conn.id_, ms, flags);
    }

    inline bool
    cancelrwtimer(augas_id cid, unsigned flags)
    {
        switch (augas_cancelrwtimer(cid, flags)) {
        case AUGAS_ERROR:
            throw error(augas_error());
        case AUGAS_NONE:
            return false;
        }
        return true;
    }

    inline bool
    cancelrwtimer(const augas_object& conn, unsigned flags)
    {
        return cancelrwtimer(conn.id_, flags);
    }

    inline augas_id
    settimer(const char* sname, unsigned ms, void* user = 0,
             void (*destroy)(void*) = 0)
    {
        int ret(augas_settimer(sname, ms, user, destroy));
        if (AUGAS_ERROR == ret)
            throw error(augas_error());
        return static_cast<augas_id>(ret);
    }

    inline bool
    resettimer(augas_id tid, unsigned ms)
    {
        switch (augas_resettimer(tid, ms)) {
        case AUGAS_ERROR:
            throw error(augas_error());
        case AUGAS_NONE:
            return false;
        }
        return true;
    }

    inline bool
    resettimer(const augas_object& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    inline bool
    canceltimer(augas_id tid)
    {
        switch (augas_canceltimer(tid)) {
        case AUGAS_ERROR:
            throw error(augas_error());
        case AUGAS_NONE:
            return false;
        }
        return true;
    }

    inline bool
    canceltimer(const augas_object& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    namespace detail {
        const char SPACE[] = " \t\n\v\f\r";
        static const char HEX[] = "0123456789ABCDEF";
    }

    inline char
    lcase(char ch)
    {
        return std::tolower(ch); // Second argument is locale.
    }

    inline char
    ucase(char ch)
    {
        return std::toupper(ch); // Second argument is locale.
    }

    inline std::string&
    ltrim(std::string& s, const char* delims = detail::SPACE)
    {
        s.erase(0, s.find_first_not_of(delims));
        return s;
    }

    inline std::string
    ltrimcopy(const std::string& s, const char* delims = detail::SPACE)
    {
        std::string::size_type pos(s.find_first_not_of(delims));
        return std::string::npos == pos
            ? std::string() : s.substr(pos);
    }

    inline std::string&
    rtrim(std::string& s, const char* delims = detail::SPACE)
    {
        std::string::size_type pos(s.find_last_not_of(delims));
        if (std::string::npos == pos)
            s.clear();
        else
            s.erase(pos + 1);
        return s;
    }

    inline std::string
    rtrimcopy(const std::string& s, const char* delims = detail::SPACE)
    {
        std::string::size_type pos(s.find_last_not_of(delims));
        return std::string::npos == pos
            ? std::string() : s.substr(0, pos + 1);
    }

    inline std::string&
    trim(std::string& s, const char* delims = detail::SPACE)
    {
        return rtrim(ltrim(s, delims), delims);
    }

    inline std::string
    trimcopy(const std::string& s, const char* delims = detail::SPACE)
    {
        return rtrimcopy(ltrimcopy(s, delims));
    }

    template <typename T, typename U, typename V>
    T
    copyif(T it, T end, U dst, V pred)
    {
        for (; it != end && pred(*it); ++it, ++dst)
            *dst = *it;
        return it;
    }

    template <typename T, typename U, typename V>
    T
    copybackif(T it, T end, U& dst, V pred)
    {
        return copyif(it, end, std::back_inserter(dst), pred);
    }

    template <typename T, typename U, typename V>
    T
    copyneq(T it, T end, U dst, V delim)
    {
        return copyif(it, end, dst,
                      std::bind2nd(std::not_equal_to<V>(), delim));
    }

    template <typename T, typename U, typename V>
    T
    copybackneq(T it, T end, U& dst, V delim)
    {
        return copyif(it, end, std::back_inserter(dst),
                      std::bind2nd(std::not_equal_to<V>(), delim));
    }

    template <typename T, typename U, typename V, typename W>
    W
    tokenise(T it, T end, U& tok, V delim, W fn)
    {
        for (; (it = copybackneq(it, end, tok, delim)) != end; ++it) {
            fn(tok);
            tok.clear();
        }
        return fn;
    }

    namespace detail {
        template <typename T>
        struct tokens {
            std::vector<T> v_;
            void
            operator ()(T& x)
            {
                v_.push_back(x);
            }
        };
    }

    template <typename T, typename U, typename V>
    std::vector<U>
    tokenise(T it, T end, U& tok, V delim)
    {
        return tokenise(it, end, tok, delim, detail::tokens<U>()).v_;
    }

    template <typename T, typename U, typename V>
    V
    splitn(T it, T end, U delim, V fn)
    {
        std::string tok;
        fn = tokenise(it, end, delim, fn);
        fn(tok);
        return fn;
    }

    template <typename T, typename U>
    std::vector<std::string>
    splitn(T it, T end, char delim)
    {
        return splitn(it, end, delim, detail::tokens<std::string>()).v_;
    }

    template <typename T, typename U, typename V>
    bool
    split2(T it, T end, U& first, U& second, V delim)
    {
        if ((it = appendneq(it, end, first, delim)) == end)
            return false;

        std::copy(++it, end, std::back_inserter(second));
        return true;
    }

    inline int
    xdigitoi(char ch)
    {
        return '0' <= ch && ch <= '9'
            ? ch - '0' : std::toupper(ch) - 'A' + 10;
    }

    template <typename T, typename U>
    U
    urlencode(T it, T end, U dst)
    {
        for (; it != end; ++it, ++dst)

            if (std::isalnum(*it))
                *dst = *it;
            else
                switch (*it) {
                case ' ':
                    *dst = '+';
                    break;
                case '-':
                case '_':
                case '.':
                case '!':
                case '~':
                case '*':
                case '\'':
                case '(':
                case ')':
                    *dst = *it;
                    break;
                default:
                    *dst++ = '%';
                    *dst++ = detail::HEX[*it / 16];
                    *dst = detail::HEX[*it % 16];
                }

        return dst;
    }

    template <typename T>
    std::string
    urlencode(T it, T end)
    {
        std::string s;
        urlencode(it, end, std::back_inserter(s));
        return s;
    }

    template <typename T, typename U>
    U
    urlpack(T it, T end, U dst)
    {
        for (bool first(true); it != end; ++it) {
            if (first)
                first = false;
            else
                *dst++ = '&';
            urlencode(it->first.begin(), it->first.end(), dst);
            *dst++ = '=';
            urlencode(it->second.begin(), it->second.end(), dst);
        }
        return dst;
    }

    template <typename T>
    std::string
    urlpack(T it, T end)
    {
        std::string s;
        urlpack(it, end, std::back_inserter(s));
        return s;
    }

    template <typename T, typename U>
    U
    urldecode(T it, T end, U dst)
    {
        for (; it != end; ++it, ++dst)
            switch (*it) {
            case '+':
                *dst = ' ';
                break;
            case '%':
                if (2 < distance(it, end) && std::isxdigit(it[1])
                    && std::isxdigit(it[2])) {
                    *dst = static_cast<char>(xdigitoi(it[1]) * 16
                                             + xdigitoi(it[2]));
                    it += 2;
                    break;
                }
            default:
                *dst = *it;
                break;
            }

        return dst;
    }

    template <typename T>
    std::string
    urldecode(T it, T end)
    {
        std::string s;
        urldecode(it, end, std::back_inserter(s));
        return s;
    }

    namespace detail {
        template <typename T>
        struct urlpairs {
            T dst_;
            explicit
            urlpairs(T dst)
                : dst_(dst)
            {
            }
            void
            operator ()(std::string& s)
            {
                std::pair<std::string, std::string> xy;
                split2(s.begin(), s.end(), xy.first, xy.second, '&');
                xy.first = urldecode(xy.first.begin(), xy.first.end());
                xy.second = urldecode(xy.second.begin(), xy.second.end());
                *dst_++ = xy;
            }
        };
    }

    template <typename T, typename U>
    U
    urlunpack(T it, T end, U dst)
    {
        std::string tok;
        splitn(it, end, tok, '=', detail::urlpairs<U>(dst));
        return dst;
    }

    template <typename T, typename U>
    std::vector<std::pair<std::string, std::string> >
    urlunpack(T it, T end)
    {
        std::vector<std::pair<std::string, std::string> > v;
        std::string tok;
        splitn(it, end, tok, '=', detail::urlpairs<U>(back_inserter(v)));
        return v;
    }
}

#endif // AUGASPP_HPP