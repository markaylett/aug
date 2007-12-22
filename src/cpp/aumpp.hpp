/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGPP_HPP
#define AUGPP_HPP

#include "aum.h"

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
# define AUM_NOTHROW throw()
#else /* NDEBUG */
# define AUM_NOTHROW
#endif /* NDEBUG */

#define AUM_WRITELOGCATCH                                   \
    catch (const std::exception& e) {                       \
        aum_writelog(AUM_LOGERROR,                          \
                     "std::exception: %s", e.what());       \
    } catch (...) {                                         \
        aum_writelog(AUM_LOGERROR, "unknown exception");    \
    } do { } while (0)

namespace aum {

    typedef std::runtime_error error;

    inline void
    writelog(int level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        aum_vwritelog(level, format, args);
        va_end(args);
    }

    inline void
    vwritelog(int level, const char* format, va_list args)
    {
        aum_vwritelog(level, format, args);
    }

    inline void
    reconfall()
    {
        if (AUM_ERROR == aum_reconfall())
            throw error(aum_error());
    }

    inline void
    stopall()
    {
        if (AUM_ERROR == aum_stopall())
            throw error(aum_error());
    }

    inline void
    post(const char* to, const char* type, struct aub_object_* ob)
    {
        if (AUM_ERROR == aum_post(to, type, ob))
            throw error(aum_error());
    }

    inline void
    dispatch(const char* to, const char* type, struct aub_object_* ob)
    {
        if (AUM_ERROR == aum_dispatch(to, type, ob))
            throw error(aum_error());
    }

    inline const char*
    getenv(const char* name, const char* def = 0)
    {
        return aum_getenv(name, def);
    }

    inline const aum_session*
    getsession()
    {
        return aum_getsession();
    }

    inline void
    shutdown(aum_id sid, unsigned flags)
    {
        if (AUM_ERROR == aum_shutdown(sid, flags))
            throw error(aum_error());
    }

    inline void
    shutdown(const aum_handle& sock, unsigned flags)
    {
        shutdown(sock.id_, flags);
    }

    inline aum_id
    tcpconnect(const char* host, const char* port, void* user = 0)
    {
        int ret(aum_tcpconnect(host, port, user));
        if (AUM_ERROR == ret)
            throw error(aum_error());
        return static_cast<aum_id>(ret);
    }

    inline aum_id
    tcplisten(const char* host, const char* port, void* user = 0)
    {
        int ret(aum_tcplisten(host, port, user));
        if (AUM_ERROR == ret)
            throw error(aum_error());
        return static_cast<aum_id>(ret);
    }

    inline void
    send(aum_id cid, const void* buf, size_t size)
    {
        if (AUM_ERROR == aum_send(cid, buf, size))
            throw error(aum_error());
    }

    inline void
    send(const aum_handle& conn, const void* buf, size_t size)
    {
        send(conn.id_, buf, size);
    }

    inline void
    sendv(aum_id cid, struct aug_blob_* blob)
    {
        if (AUM_ERROR == aum_sendv(cid, blob))
            throw error(aum_error());
    }

    inline void
    sendv(const aum_handle& conn, struct aug_blob_* blob)
    {
        sendv(conn.id_, blob);
    }

    inline void
    setrwtimer(aum_id cid, unsigned ms, unsigned flags)
    {
        if (AUM_ERROR == aum_setrwtimer(cid, ms, flags))
            throw error(aum_error());
    }

    inline void
    setrwtimer(const aum_handle& conn, unsigned ms, unsigned flags)
    {
        setrwtimer(conn.id_, ms, flags);
    }

    inline bool
    resetrwtimer(aum_id cid, unsigned ms, unsigned flags)
    {
        switch (aum_resetrwtimer(cid, ms, flags)) {
        case AUM_ERROR:
            throw error(aum_error());
        case AUM_NONE:
            return false;
        }
        return true;
    }

    inline bool
    retsetrwtimer(const aum_handle& conn, unsigned ms, unsigned flags)
    {
        return resetrwtimer(conn.id_, ms, flags);
    }

    inline bool
    cancelrwtimer(aum_id cid, unsigned flags)
    {
        switch (aum_cancelrwtimer(cid, flags)) {
        case AUM_ERROR:
            throw error(aum_error());
        case AUM_NONE:
            return false;
        }
        return true;
    }

    inline bool
    cancelrwtimer(const aum_handle& conn, unsigned flags)
    {
        return cancelrwtimer(conn.id_, flags);
    }

    inline aum_id
    settimer(unsigned ms, struct aub_object_* ob)
    {
        int ret(aum_settimer(ms, ob));
        if (AUM_ERROR == ret)
            throw error(aum_error());
        return static_cast<aum_id>(ret);
    }

    inline bool
    resettimer(aum_id tid, unsigned ms)
    {
        switch (aum_resettimer(tid, ms)) {
        case AUM_ERROR:
            throw error(aum_error());
        case AUM_NONE:
            return false;
        }
        return true;
    }

    inline bool
    resettimer(const aum_handle& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    inline bool
    canceltimer(aum_id tid)
    {
        switch (aum_canceltimer(tid)) {
        case AUM_ERROR:
            throw error(aum_error());
        case AUM_NONE:
            return false;
        }
        return true;
    }

    inline bool
    canceltimer(const aum_handle& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    inline void
    setsslclient(aum_id cid, const char* ctx)
    {
        if (AUM_ERROR == aum_setsslclient(cid, ctx))
            throw error(aum_error());
    }

    inline void
    setsslclient(const aum_handle& conn, const char* ctx)
    {
        setsslclient(conn.id_, ctx);
    }

    inline void
    setsslserver(aum_id cid, const char* ctx)
    {
        if (AUM_ERROR == aum_setsslserver(cid, ctx))
            throw error(aum_error());
    }

    inline void
    setsslserver(const aum_handle& conn, const char* ctx)
    {
        setsslserver(conn.id_, ctx);
    }

    class handle {
        const aum_handle& handle_;
    public:
        explicit
        handle(const aum_handle& handle)
            : handle_(handle)
        {
        }
        void
        setuser(void* user)
        {
            const_cast<aum_handle&>(handle_).user_ = user;
        }
        aum_id
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
        operator const aum_handle&() const
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
                 struct aub_object_* ob) = 0;

        virtual void
        do_closed(const handle& sock) = 0;

        virtual void
        do_teardown(const handle& sock) = 0;

        virtual bool
        do_accepted(handle& sock, const char* addr, unsigned short port) = 0;

        virtual void
        do_connected(handle& sock, const char* addr, unsigned short port) = 0;

        virtual void
        do_data(const handle& sock, const void* buf, size_t size) = 0;

        virtual void
        do_rdexpire(const handle& sock, unsigned& ms) = 0;

        virtual void
        do_wrexpire(const handle& sock, unsigned& ms) = 0;

        virtual void
        do_expire(const handle& timer, unsigned& ms) = 0;

        virtual bool
        do_authcert(const handle& sock, const char* subject,
                    const char* issuer) = 0;

    public:
        virtual
        ~session_base() AUM_NOTHROW
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
        event(const char* from, const char* type, struct aub_object_* ob)
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
        accepted(handle& sock, const char* addr, unsigned short port)
        {
            return do_accepted(sock, addr, port);
        }
        void
        connected(handle& sock, const char* addr, unsigned short port)
        {
            do_connected(sock, addr, port);
        }
        void
        data(const handle& sock, const void* buf, size_t size)
        {
            do_data(sock, buf, size);
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
        bool
        authcert(const handle& sock, const char* subject, const char* issuer)
        {
            return do_authcert(sock, subject, issuer);
        }
    };

    class basic_session : public session_base {
        void
        do_reconf()
        {
        }
        void
        do_event(const char* from, const char* type, struct aub_object_* ob)
        {
        }
        void
        do_closed(const handle& sock)
        {
        }
        void
        do_teardown(const handle& sock)
        {
            aum_writelog(AUM_LOGINFO, "teardown defaulting to shutdown");
            shutdown(sock, 0);
        }
        bool
        do_accepted(handle& sock, const char* addr, unsigned short port)
        {
            return true;
        }
        void
        do_connected(handle& sock, const char* addr, unsigned short port)
        {
        }
        void
        do_data(const handle& sock, const void* buf, size_t size)
        {
        }
        void
        do_rdexpire(const handle& sock, unsigned& ms)
        {
        }
        void
        do_wrexpire(const handle& sock, unsigned& ms)
        {
        }
        void
        do_expire(const handle& timer, unsigned& ms)
        {
        }
        bool
        do_authcert(const handle& sock, const char* subject,
                    const char* issuer)
        {
            return true;
        }
    public:
        virtual
        ~basic_session() AUM_NOTHROW
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
            aum_writelog(AUM_LOGINFO, "creating factory: module=[%s]",
                         module);
        }
        session_base*
        create(const char* sname)
        {
            aum_writelog(AUM_LOGINFO, "creating session: name=[%s]",
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
        static int
        result(bool x)
        {
            return x ? AUM_OK : AUM_ERROR;
        }
        static void
        stop() AUM_NOTHROW
        {
            delete getbase();
        }
        static int
        start(aum_session* session) AUM_NOTHROW
        {
            try {
                session->user_ = factory_->create(session->name_);
                return result(getbase()->start(session->name_));
            } AUM_WRITELOGCATCH;
            return AUM_ERROR;
        }
        static void
        reconf() AUM_NOTHROW
        {
            try {
                getbase()->reconf();
            } AUM_WRITELOGCATCH;
        }
        static void
        event(const char* from, const char* type,
              struct aub_object_* ob) AUM_NOTHROW
        {
            try {
                getbase()->event(from, type, ob);
            } AUM_WRITELOGCATCH;
        }
        static void
        closed(const aum_handle* sock) AUM_NOTHROW
        {
            try {
                getbase()->closed(handle(*sock));
            } AUM_WRITELOGCATCH;
        }
        static void
        teardown(const aum_handle* sock) AUM_NOTHROW
        {
            try {
                getbase()->teardown(handle(*sock));
            } AUM_WRITELOGCATCH;
        }
        static int
        accepted(aum_handle* sock, const char* addr,
                 unsigned short port) AUM_NOTHROW
        {
            try {
                handle h(*sock);
                return result(getbase()->accepted(h, addr, port));
            } AUM_WRITELOGCATCH;
            return AUM_ERROR;
        }
        static void
        connected(aum_handle* sock, const char* addr,
                  unsigned short port) AUM_NOTHROW
        {
            try {
                handle h(*sock);
                getbase()->connected(h, addr, port);
            } AUM_WRITELOGCATCH;
        }
        static void
        data(const aum_handle* sock, const void* buf,
             size_t size) AUM_NOTHROW
        {
            try {
                getbase()->data(handle(*sock), buf, size);
            } AUM_WRITELOGCATCH;
        }
        static void
        rdexpire(const aum_handle* sock, unsigned* ms) AUM_NOTHROW
        {
            try {
                getbase()->rdexpire(handle(*sock), *ms);
            } AUM_WRITELOGCATCH;
        }
        static void
        wrexpire(const aum_handle* sock, unsigned* ms) AUM_NOTHROW
        {
            try {
                getbase()->wrexpire(handle(*sock), *ms);
            } AUM_WRITELOGCATCH;
        }
        static void
        expire(const aum_handle* timer, unsigned* ms) AUM_NOTHROW
        {
            try {
                getbase()->expire(handle(*timer), *ms);
            } AUM_WRITELOGCATCH;
        }
        static int
        authcert(const aum_handle* sock, const char* subject,
                 const char* issuer)
        {
            try {
                handle h(*sock);
                return result(getbase()->authcert(h, subject, issuer));
            } AUM_WRITELOGCATCH;
            return AUM_ERROR;
        }

    public:
        static const aum_module*
        init(const char* name) AUM_NOTHROW
        {
            static const aum_module local = {
                stop,
                start,
                reconf,
                event,
                closed,
                teardown,
                accepted,
                connected,
                data,
                rdexpire,
                wrexpire,
                expire,
                authcert
            };
            try {
                factory_ = new T(name);
                return &local;
            } AUM_WRITELOGCATCH;
            return 0; // Error.
        }
        static void
        term() AUM_NOTHROW
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
}

#endif // AUGPP_HPP
