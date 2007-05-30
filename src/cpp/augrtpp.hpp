/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_HPP
#define AUGRTPP_HPP

#include "augrt.h"

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

#if !defined(AUGRT_NOTHROW)
# define AUGRT_NOTHROW throw()
#endif // !AUGRT_NOTHROW

#define AUGRT_WRITELOGCATCH                                     \
    catch (const std::exception& e) {                           \
        augrt_writelog(AUGRT_LOGERROR,                          \
                       "std::exception: %s", e.what());         \
    } catch (...) {                                             \
        augrt_writelog(AUGRT_LOGERROR, "unknown exception");    \
    } do { } while (0)

namespace augrt {

    typedef std::runtime_error error;

    inline void
    writelog(int level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        augrt_vwritelog(level, format, args);
        va_end(args);
    }

    inline void
    vwritelog(int level, const char* format, va_list args)
    {
        augrt_vwritelog(level, format, args);
    }

    inline void
    reconfall()
    {
        if (AUGRT_ERROR == augrt_reconfall())
            throw error(augrt_error());
    }

    inline void
    stopall()
    {
        if (AUGRT_ERROR == augrt_stopall())
            throw error(augrt_error());
    }

    inline void
    post(const char* to, const char* type, const augrt_var& var)
    {
        if (AUGRT_ERROR == augrt_post(to, type, &var))
            throw error(augrt_error());
    }

    inline void
    dispatch(const char* to, const char* type, const void* user, size_t size)
    {
        if (AUGRT_ERROR == augrt_dispatch(to, type, user, size))
            throw error(augrt_error());
    }

    inline const char*
    getenv(const char* name, const char* def = 0)
    {
        return augrt_getenv(name, def);
    }

    inline const augrt_serv*
    getserv()
    {
        return augrt_getserv();
    }

    inline void
    shutdown(augrt_id sid)
    {
        if (AUGRT_ERROR == augrt_shutdown(sid))
            throw error(augrt_error());
    }

    inline void
    shutdown(const augrt_object& sock)
    {
        shutdown(sock.id_);
    }

    inline augrt_id
    tcpconnect(const char* host, const char* port, void* user = 0)
    {
        int ret(augrt_tcpconnect(host, port, user));
        if (AUGRT_ERROR == ret)
            throw error(augrt_error());
        return static_cast<augrt_id>(ret);
    }

    inline augrt_id
    tcplisten(const char* host, const char* port, void* user = 0)
    {
        int ret(augrt_tcplisten(host, port, user));
        if (AUGRT_ERROR == ret)
            throw error(augrt_error());
        return static_cast<augrt_id>(ret);
    }

    inline void
    send(augrt_id cid, const void* buf, size_t size)
    {
        if (AUGRT_ERROR == augrt_send(cid, buf, size))
            throw error(augrt_error());
    }

    inline void
    send(const augrt_object& conn, const void* buf, size_t size)
    {
        send(conn.id_, buf, size);
    }

    inline void
    sendv(augrt_id cid, const augrt_var& var)
    {
        if (AUGRT_ERROR == augrt_sendv(cid, &var))
            throw error(augrt_error());
    }

    inline void
    sendv(const augrt_object& conn, const augrt_var& var)
    {
        sendv(conn.id_, var);
    }

    inline void
    setrwtimer(augrt_id cid, unsigned ms, unsigned flags)
    {
        if (AUGRT_ERROR == augrt_setrwtimer(cid, ms, flags))
            throw error(augrt_error());
    }

    inline void
    setrwtimer(const augrt_object& conn, unsigned ms, unsigned flags)
    {
        setrwtimer(conn.id_, ms, flags);
    }

    inline bool
    resetrwtimer(augrt_id cid, unsigned ms, unsigned flags)
    {
        switch (augrt_resetrwtimer(cid, ms, flags)) {
        case AUGRT_ERROR:
            throw error(augrt_error());
        case AUGRT_NONE:
            return false;
        }
        return true;
    }

    inline bool
    retsetrwtimer(const augrt_object& conn, unsigned ms, unsigned flags)
    {
        return resetrwtimer(conn.id_, ms, flags);
    }

    inline bool
    cancelrwtimer(augrt_id cid, unsigned flags)
    {
        switch (augrt_cancelrwtimer(cid, flags)) {
        case AUGRT_ERROR:
            throw error(augrt_error());
        case AUGRT_NONE:
            return false;
        }
        return true;
    }

    inline bool
    cancelrwtimer(const augrt_object& conn, unsigned flags)
    {
        return cancelrwtimer(conn.id_, flags);
    }

    inline augrt_id
    settimer(unsigned ms, const augrt_var& var)
    {
        int ret(augrt_settimer(ms, &var));
        if (AUGRT_ERROR == ret)
            throw error(augrt_error());
        return static_cast<augrt_id>(ret);
    }

    inline bool
    resettimer(augrt_id tid, unsigned ms)
    {
        switch (augrt_resettimer(tid, ms)) {
        case AUGRT_ERROR:
            throw error(augrt_error());
        case AUGRT_NONE:
            return false;
        }
        return true;
    }

    inline bool
    resettimer(const augrt_object& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    inline bool
    canceltimer(augrt_id tid)
    {
        switch (augrt_canceltimer(tid)) {
        case AUGRT_ERROR:
            throw error(augrt_error());
        case AUGRT_NONE:
            return false;
        }
        return true;
    }

    inline bool
    canceltimer(const augrt_object& timer, unsigned ms)
    {
        return resettimer(timer.id_, ms);
    }

    inline void
    setsslclient(augrt_id cid, const char* ctx)
    {
        if (AUGRT_ERROR == augrt_setsslclient(cid, ctx))
            throw error(augrt_error());
    }

    inline void
    setsslclient(const augrt_object& conn, const char* ctx)
    {
        setsslclient(conn.id_, ctx);
    }

    inline void
    setsslserver(augrt_id cid, const char* ctx)
    {
        if (AUGRT_ERROR == augrt_setsslserver(cid, ctx))
            throw error(augrt_error());
    }

    inline void
    setsslserver(const augrt_object& conn, const char* ctx)
    {
        setsslserver(conn.id_, ctx);
    }

    namespace detail {
        class stringtype {
            static int
            destroy(void* arg) AUGRT_NOTHROW
            {
                delete static_cast<std::string*>(arg);
                return 0;
            }
            static const void*
            buf(void* arg, size_t* size) AUGRT_NOTHROW
            {
                std::string* s(static_cast<std::string*>(arg));
                if (size)
                    *size = s->size();
                return s->data();
            }
        public:
            static const augrt_vartype&
            get()
            {
                static const augrt_vartype local = {
                    destroy,
                    buf
                };
                return local;
            }
        };
    }

    inline aug_var&
    stringvar(aug_var& var, const std::auto_ptr<std::string>& ptr)
    {
        var.type_ = &detail::stringtype::get();
        var.arg_ = ptr.get();
        return var;
    }

    class object {
        const augrt_object& object_;
    public:
        explicit
        object(const augrt_object& object)
            : object_(object)
        {
        }
        void
        setuser(void* user)
        {
            const_cast<augrt_object&>(object_).user_ = user;
        }
        augrt_id
        id() const
        {
            return object_.id_;
        }
        void*
        user() const
        {
            return object_.user_;
        }
        template <typename T>
        T*
        user() const
        {
            return static_cast<T*>(object_.user_);
        }
        operator const augrt_object&() const
        {
            return object_;
        }
    };

    class serv_base {
        virtual bool
        do_start(const char* sname) = 0;

        virtual void
        do_reconf() = 0;

        virtual void
        do_event(const char* from, const char* type, const void* user,
                 size_t size) = 0;

        virtual void
        do_closed(const object& sock) = 0;

        virtual void
        do_teardown(const object& sock) = 0;

        virtual bool
        do_accepted(object& sock, const char* addr, unsigned short port) = 0;

        virtual void
        do_connected(object& sock, const char* addr, unsigned short port) = 0;

        virtual void
        do_data(const object& sock, const void* buf, size_t size) = 0;

        virtual void
        do_rdexpire(const object& sock, unsigned& ms) = 0;

        virtual void
        do_wrexpire(const object& sock, unsigned& ms) = 0;

        virtual void
        do_expire(const object& timer, unsigned& ms) = 0;

        virtual bool
        do_authcert(const object& sock, const char* subject,
                    const char* issuer) = 0;

    public:
        virtual
        ~serv_base() AUGRT_NOTHROW
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
        event(const char* from, const char* type, const void* user,
              size_t size)
        {
            do_event(from, type, user, size);
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
        accepted(object& sock, const char* addr, unsigned short port)
        {
            return do_accepted(sock, addr, port);
        }
        void
        connected(object& sock, const char* addr, unsigned short port)
        {
            do_connected(sock, addr, port);
        }
        void
        data(const object& sock, const void* buf, size_t size)
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
        bool
        authcert(const object& sock, const char* subject, const char* issuer)
        {
            return do_authcert(sock, subject, issuer);
        }
    };

    class basic_serv : public serv_base {
        void
        do_reconf()
        {
        }
        void
        do_event(const char* from, const char* type, const void* user,
                 size_t size)
        {
        }
        void
        do_closed(const object& sock)
        {
        }
        void
        do_teardown(const object& sock)
        {
            augrt_writelog(AUGRT_LOGINFO, "teardown defaulting to shutdown");
            shutdown(sock);
        }
        bool
        do_accepted(object& sock, const char* addr, unsigned short port)
        {
            return true;
        }
        void
        do_connected(object& sock, const char* addr, unsigned short port)
        {
        }
        void
        do_data(const object& sock, const void* buf, size_t size)
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
        bool
        do_authcert(const object& sock, const char* subject,
                    const char* issuer)
        {
            return true;
        }
    public:
        virtual
        ~basic_serv() AUGRT_NOTHROW
        {
        }
    };

    struct nil {
        static serv_base*
        create(const char* sname)
        {
            return 0;
        }
    };

    template <typename headT, typename tailT>
    struct cons {
        static serv_base*
        create(const char* sname)
        {
            serv_base* p = headT::create(sname);
            return p ? p : tailT::create(sname);
        }
    };

    template <typename listT>
    struct basic_factory {
        explicit
        basic_factory(const char* module)
        {
            augrt_writelog(AUGRT_LOGINFO, "creating factory: module=[%s]",
                           module);
        }
        serv_base*
        create(const char* sname)
        {
            augrt_writelog(AUGRT_LOGINFO, "creating service: name=[%s]",
                           sname);
            return listT::create(sname);
        }
    };

    template <typename T>
    class basic_module {
        static T* factory_;
        static serv_base*
        getbase()
        {
            return static_cast<serv_base*>(getserv()->user_);
        }
        static int
        result(bool x)
        {
            return x ? AUGRT_OK : AUGRT_ERROR;
        }
        static void
        stop() AUGRT_NOTHROW
        {
            delete getbase();
        }
        static int
        start(augrt_serv* serv) AUGRT_NOTHROW
        {
            try {
                serv->user_ = factory_->create(serv->name_);
                return result(getbase()->start(serv->name_));
            } AUGRT_WRITELOGCATCH;
            return AUGRT_ERROR;
        }
        static void
        reconf() AUGRT_NOTHROW
        {
            try {
                getbase()->reconf();
            } AUGRT_WRITELOGCATCH;
        }
        static void
        event(const char* from, const char* type, const void* user,
              size_t size) AUGRT_NOTHROW
        {
            try {
                getbase()->event(from, type, user, size);
            } AUGRT_WRITELOGCATCH;
        }
        static void
        closed(const augrt_object* sock) AUGRT_NOTHROW
        {
            try {
                getbase()->closed(object(*sock));
            } AUGRT_WRITELOGCATCH;
        }
        static void
        teardown(const augrt_object* sock) AUGRT_NOTHROW
        {
            try {
                getbase()->teardown(object(*sock));
            } AUGRT_WRITELOGCATCH;
        }
        static int
        accepted(augrt_object* sock, const char* addr,
                 unsigned short port) AUGRT_NOTHROW
        {
            try {
                object o(*sock);
                return result(getbase()->accepted(o, addr, port));
            } AUGRT_WRITELOGCATCH;
            return AUGRT_ERROR;
        }
        static void
        connected(augrt_object* sock, const char* addr,
                  unsigned short port) AUGRT_NOTHROW
        {
            try {
                object o(*sock);
                getbase()->connected(o, addr, port);
            } AUGRT_WRITELOGCATCH;
        }
        static void
        data(const augrt_object* sock, const void* buf,
             size_t size) AUGRT_NOTHROW
        {
            try {
                getbase()->data(object(*sock), buf, size);
            } AUGRT_WRITELOGCATCH;
        }
        static void
        rdexpire(const augrt_object* sock, unsigned* ms) AUGRT_NOTHROW
        {
            try {
                getbase()->rdexpire(object(*sock), *ms);
            } AUGRT_WRITELOGCATCH;
        }
        static void
        wrexpire(const augrt_object* sock, unsigned* ms) AUGRT_NOTHROW
        {
            try {
                getbase()->wrexpire(object(*sock), *ms);
            } AUGRT_WRITELOGCATCH;
        }
        static void
        expire(const augrt_object* timer, unsigned* ms) AUGRT_NOTHROW
        {
            try {
                getbase()->expire(object(*timer), *ms);
            } AUGRT_WRITELOGCATCH;
        }
        static int
        authcert(const augrt_object* sock, const char* subject,
                 const char* issuer)
        {
            try {
                object o(*sock);
                return result(getbase()->authcert(o, subject, issuer));
            } AUGRT_WRITELOGCATCH;
            return AUGRT_ERROR;
        }

    public:
        static const struct augrt_module*
        init(const char* name) AUGRT_NOTHROW
        {
            static const augrt_module local = {
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
            } AUGRT_WRITELOGCATCH;
            return 0; // Error.
        }
        static void
        term() AUGRT_NOTHROW
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

#endif // AUGRTPP_HPP
