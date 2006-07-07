/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CONN_HPP
#define AUGNETPP_CONN_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/conn.h"

#include "augutil/list.h"

#include "augsys/errno.h"
#include "augsys/log.h"

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
    public:
        typedef struct aug_conns ctype;
    private:

        friend class conn;

        struct aug_conns conns_;

        conns(const conns&);

        conns&
        operator =(const conns&);

    public:
        ~conns() NOTHROW
        {
            if (-1 == aug_freeconns(&conns_))
                aug_perrinfo("aug_freeconns() failed");
        }

        conns()
        {
            AUG_INIT(&conns_);
        }

        operator struct aug_conns&()
        {
            return conns_;
        }

        operator const struct aug_conns&() const
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
        poll(const struct aug_var* arg, int id, struct aug_conns* conns)
        {
            try {
                poll_base* ptr = static_cast<poll_base*>(aug_getvarp(arg));
                return ptr->poll(id, *conns) ? 1 : 0;
            } AUG_SETERRINFOCATCH;
            return 0; /* false */
        }
    }

    inline void
    insertconn(struct aug_conns& conns, fdref ref, poll_base& action)
    {
        var v(&action);
        if (-1 == aug_insertconn(&conns, ref.get(), detail::poll, cptr(v)))
            throwerrinfo("aug_insertconn() failed");
    }

    inline void
    removeconn(struct aug_conns& conns, fdref ref)
    {
        if (-1 == aug_removeconn(&conns, ref.get()))
            throwerrinfo("aug_removeconn() failed");
    }

    inline void
    processconns(struct aug_conns& conns)
    {
        if (-1 == aug_processconns(&conns))
            throwerrinfo("aug_processconns() failed");
    }
}

#endif // AUGNETPP_CONN_HPP
