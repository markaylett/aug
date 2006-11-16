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

    class conncb_base {

        virtual bool
        do_callback(int fd, aug_conns& conns) = 0;

    public:
        virtual
        ~conncb_base() AUG_NOTHROW
        {
        }

        bool
        callback(int fd, aug_conns& conns)
        {
            return do_callback(fd, conns);
        }

        bool
        operator ()(int fd, aug_conns& conns)
        {
            return do_callback(fd, conns);
        }
    };

    class conns {
    public:
        typedef aug_conns ctype;
    private:

        friend class conn;

        aug_conns conns_;

        conns(const conns&);

        conns&
        operator =(const conns&);

    public:
        ~conns() AUG_NOTHROW
        {
            if (-1 == aug_freeconns(&conns_))
                perrinfo("aug_freeconns() failed");
        }

        conns()
        {
            AUG_INIT(&conns_);
        }

        operator aug_conns&()
        {
            return conns_;
        }

        operator const aug_conns&() const
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
        conncb(int id, const aug_var* arg, aug_conns* conns)
        {
            try {
                conncb_base* ptr = static_cast<
                    conncb_base*>(aug_getvarp(arg));
                return ptr->callback(id, *conns) ? 1 : 0;
            } AUG_PERRINFOCATCH;

            /**
               Do not remove the connection unless explicitly asked to.
            */

            return 1;
        }
    }

    inline void
    insertconn(aug_conns& conns, fdref ref, conncb_base& cb)
    {
        var v(&cb);
        verify(aug_insertconn(&conns, ref.get(), detail::conncb, cptr(v)));
    }

    inline void
    removeconn(aug_conns& conns, fdref ref)
    {
        verify(aug_removeconn(&conns, ref.get()));
    }

    inline void
    processconns(aug_conns& conns)
    {
        verify(aug_processconns(&conns));
    }
}

#endif // AUGNETPP_CONN_HPP
