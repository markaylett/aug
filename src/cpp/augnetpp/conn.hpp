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

        operator struct aug_conns&()
        {
            return conns_;
        }
        struct aug_conns&
        get()
        {
            return conns_;
        }

        bool
        empty() const;
    };

    AUGNETPP_API void
    insertconn(struct aug_conns& conns, fdref ref, poll_base& action);

    inline void
    removeconn(struct aug_conns& conns, fdref ref)
    {
        if (-1 == aug_removeconn(&conns, ref.get()))
            error("aug_removeconn() failed");
    }

    inline void
    processconns(struct aug_conns& conns)
    {
        if (-1 == aug_processconns(&conns))
            error("aug_processconns() failed");
    }
}

#endif // AUGNETPP_CONN_HPP
