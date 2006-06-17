/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CONN_HPP
#define AUGNETPP_CONN_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/conn.h"

namespace aug {

    class AUGNETPP_API poll_base {

        virtual bool
        do_poll(int fd, struct aug_conns& conns) = 0;

    public:
        virtual
        ~poll_base() NOTHROW;

        bool
        poll(int fd, struct aug_conns& conns)
        {
            return do_poll(fd, conns);
        }
    };

    class AUGNETPP_API conns {

        friend class conn;

        struct aug_conns conns_;

        conns(const conns&);

        conns&
        operator =(const conns&);

    public:
        ~conns() NOTHROW;

        conns();

        void
        insert(fdref ref, poll_base& action);

        void
        remove(fdref ref)
        {
            if (-1 == aug_removeconn(&conns_, ref.get()))
                error("aug_removeconn() failed");
        }

        void
        process()
        {
            if (-1 == aug_processconns(&conns_))
                error("aug_processconns() failed");
        }

        bool
        empty() const;
    };
}

#endif // AUGNETPP_CONN_HPP
