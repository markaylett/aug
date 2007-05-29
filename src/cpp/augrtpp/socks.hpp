/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SOCKS_HPP
#define AUGRTPP_SOCKS_HPP

#include "augrtpp/serv.hpp"
#include "augrtpp/sock.hpp"

#include <map>

namespace aug {

    class socks {

        typedef std::map<augas_id, int, std::greater<augas_id> > idtofd;

        std::map<int, sockptr> socks_;
        idtofd idtofd_;

        socks(const socks& rhs);

        socks&
        operator =(const socks& rhs);

    public:
        ~socks() AUG_NOTHROW;

        socks()
        {
        }
        bool
        send(augas_id cid, const void* buf, size_t size);

        bool
        sendv(augas_id cid, const aug_var& var);

        void
        clear();

        void
        erase(const sock_base& sock);

        void
        insert(const sockptr& sock);

        void
        update(const sockptr& sock, fdref prev);

        void
        teardown();

        sockptr
        getbyfd(fdref fd) const;

        sockptr
        getbyid(augas_id id) const;

        bool
        empty() const;
    };

    class scoped_insert {

        socks& socks_;
        sockptr sock_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != sock_)
                socks_.erase(*sock_);
        }
        scoped_insert(socks& socks, const sockptr& sock)
            : socks_(socks),
              sock_(sock)
        {
            socks.insert(sock);
        }
        void
        commit()
        {
            sock_ = sockptr();
        }
    };
}

#endif // AUGRTPP_SOCKS_HPP
