/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CONN_HPP
#define AUGNETPP_CONN_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/conn.h"

#include "augutil/list.h"

#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/string.h" // aug_perror()

namespace aug {

    class AUGNETPP_API poll_base {

        virtual bool
        do_poll(int fd, struct aug_conns& conns) = 0;

    public:
        virtual
        ~poll_base() NOTHROW
        {
        }

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
        ~conns() NOTHROW
        {
            if (-1 == aug_freeconns(&conns_))
                aug_perror("aug_freeconns() failed");
        }

        conns()
        {
            AUG_INIT(&conns_);
        }

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
        empty() const
        {
            return AUG_EMPTY(&conns_);
        }
    };

    namespace detail {

        inline int
        poll(void* arg, int id, struct aug_conns* conns)
        {
            try {
                poll_base* ptr = static_cast<poll_base*>(arg);
                return ptr->poll(id, *conns) ? 1 : 0;
            } AUG_CATCHRETURN 0; /* false */
        }
    }

    inline void
    insertconn(struct aug_conns& conns, fdref ref, poll_base& action)
    {
        if (-1 == aug_insertconn(&conns, ref.get(), detail::poll, &action))
            throwerror("aug_insertconn() failed");
    }

    inline void
    removeconn(struct aug_conns& conns, fdref ref)
    {
        if (-1 == aug_removeconn(&conns, ref.get()))
            throwerror("aug_removeconn() failed");
    }

    inline void
    processconns(struct aug_conns& conns)
    {
        if (-1 == aug_processconns(&conns))
            throwerror("aug_processconns() failed");
    }
}

#endif // AUGNETPP_CONN_HPP
