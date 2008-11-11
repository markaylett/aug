/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGPP_HPP
#define AUGPP_HPP

#include "augmod.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <functional>

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
    post(const char* to, const char* type, struct aug_object_* ob)
    {
        if (mod_post(to, type, ob) < 0)
            throw error(mod_error());
    }

    inline void
    dispatch(const char* to, const char* type, struct aug_object_* ob)
    {
        if (mod_dispatch(to, type, ob) < 0)
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
    sendv(mod_id cid, struct aug_blob_* blob)
    {
        if (mod_sendv(cid, blob) < 0)
            throw error(mod_error());
    }

    inline void
    sendv(const mod_handle& conn, struct aug_blob_* blob)
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
    settimer(unsigned ms, struct aug_object_* ob)
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
        do_event(const char* from, const char* type,
                 struct aug_object_* ob) = 0;

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
        event(const char* from, const char* type, struct aug_object_* ob)
        {
            do_event(from, type, ob);
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
        do_event(const char* from, const char* type, struct aug_object_* ob)
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
        event(const char* from, const char* type,
              struct aug_object_* ob) MOD_NOTHROW
        {
            try {
                getbase()->event(from, type, ob);
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
        template <typename T, typename U>
        class tokens {
            T it_;
        public:
            explicit
            tokens(T it)
                : it_(it)
            {
            }
            void
            operator ()(U& x)
            {
                *it_++ = x;
            }
        };
        template <typename T, typename U>
        tokens<U, T>
        maketokens(U it)
        {
            return tokens<U, T>(it);
        }
    }

    template <typename T, typename U, typename V>
    std::vector<U>
    tokenise(T it, T end, U& tok, V delim)
    {
        std::vector<U> v;
        tokenise(it, end, tok, delim,
                 detail::maketokens<U>(std::back_inserter(v)));
        return v;
    }

    template <typename T, typename U, typename V>
    V
    splitn(T it, T end, U delim, V fn)
    {
        std::string tok;
        fn = tokenise(it, end, tok, delim, fn);
        fn(tok);
        return fn;
    }

    template <typename T>
    std::vector<std::string>
    splitn(T it, T end, char delim)
    {
        std::vector<std::string> v;
        splitn(it, end, delim,
               detail::maketokens<std::string>(std::back_inserter(v)));
        return v;
    }

    template <typename T, typename U, typename V>
    bool
    split2(T it, T end, U& first, U& second, V delim)
    {
        if ((it = copybackneq(it, end, first, delim)) == end)
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
            T it_;
            explicit
            urlpairs(T it)
                : it_(it)
            {
            }
            void
            operator ()(std::string& s)
            {
                std::pair<std::string, std::string> xy;
                split2(s.begin(), s.end(), xy.first, xy.second, '=');
                xy.first = urldecode(xy.first.begin(), xy.first.end());
                xy.second = urldecode(xy.second.begin(), xy.second.end());
                *it_++ = xy;
            }
        };
        template <typename T>
        urlpairs<T>
        makeurlpairs(T it)
        {
            return urlpairs<T>(it);
        }
    }

    template <typename T, typename U>
    U
    urlunpack(T it, T end, U dst)
    {
        return splitn(it, end, '&', detail::makeurlpairs(dst)).it_;
    }

    template <typename T>
    std::vector<std::pair<std::string, std::string> >
    urlunpack(T it, T end)
    {
        std::vector<std::pair<std::string, std::string> > v;
        splitn(it, end, '&',
               detail::makeurlpairs(std::back_inserter(v)));
        return v;
    }

    template<typename T>
    bool
    getvalue(const std::map<std::string, std::string>& pairs,
             const std::string& key, T& value)
    {
        std::map<std::string, std::string>
            ::const_iterator it(pairs.find(key));
        if (it == pairs.end())
            return false;

        std::istringstream ss(it->second);
        if (!(ss >> value))
            throw std::bad_cast();

        return true;
    }

    template<typename T>
    T
    getvalue(const std::map<std::string, std::string>& pairs,
             const std::string& key)
    {
        T value = T();
        getvalue(pairs, key, value);
        return value;
    }
}}

#endif // AUGPP_HPP
