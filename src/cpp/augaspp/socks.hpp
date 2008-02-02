/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SOCKS_HPP
#define AUGRTPP_SOCKS_HPP

#include "augaspp/session.hpp"
#include "augaspp/sock.hpp"

#include "augob/blob.h"

#include <map>

struct timeval;

namespace aug {

    class socks {

        typedef std::map<mod_id, int, std::greater<mod_id> > idtofd;

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
        send(mod_id cid, const void* buf, size_t size, const timeval& now);

        bool
        sendv(mod_id cid, blobref ref, const timeval& now);

        void
        clear();

        void
        erase(const sock_base& sock);

        void
        insert(const sockptr& sock);

        void
        update(const sockptr& sock, fdref prev);

        void
        teardown(const timeval& tv);

        sockptr
        getbyfd(fdref fd) const;

        sockptr
        getbyid(mod_id id) const;

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
