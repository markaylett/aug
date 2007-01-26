/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MANAGER_HPP
#define AUGAS_MANAGER_HPP

#include "augas/conn.hpp"

#include <map>

namespace augas {

    class options;

    class manager {

        typedef std::map<std::string, moduleptr> modules;
        typedef std::map<std::string, sessptr> sesss;
        typedef std::map<augas_id, int, std::greater<augas_id> > idtofd;
        typedef std::map<int, sockptr> socks;

        modules modules_;
        sesss sesss_;
        socks socks_;
        idtofd idtofd_;

        manager(const manager& rhs);

        manager&
        operator =(const manager& rhs);

        void
        insert(const std::string& name, const sessptr& sess);

    public:
        manager()
        {
        }
        void
        clear();

        void
        erase(const sock_base& sock);

        void
        insert(const sockptr& sock);

        void
        update(const sockptr& sock, aug::fdref prev);

        void
        load(const char* rundir, const options& options,
             const augas_host& host);

        bool
        sendall(aug::mplexer& mplexer, augas_id cid, const char* sname,
                const char* buf, size_t size);

        bool
        sendself(aug::mplexer& mplexer, augas_id cid, const char* buf,
                 size_t size);

        void
        sendother(aug::mplexer& mplexer, augas_id cid, const char* sname,
                  const char* buf, size_t size);

        void
        teardown();

        void
        reconf() const;

        sockptr
        getbyfd(aug::fdref fd) const;

        sockptr
        getbyid(augas_id id) const;

        sessptr
        getsess(const std::string& name) const;

        bool
        empty() const;
    };

    class scoped_insert {

        manager& manager_;
        sockptr sock_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != sock_)
                manager_.erase(*sock_);
        }
        scoped_insert(manager& manager, const sockptr& sock)
            : manager_(manager),
              sock_(sock)
        {
            manager.insert(sock);
        }
        void
        commit()
        {
            sock_ = sockptr();
        }
    };
}

#endif // AUGAS_MANAGER_HPP
