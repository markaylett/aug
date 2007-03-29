/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_CONN_HPP
#define AUGAS_CONN_HPP

#include "augas/rwtimer.hpp"
#include "augas/object.hpp"

#include "augnetpp.hpp"

namespace augas {

    class buffer;

    enum connphase {
        CONNECTING,
        ESTABLISHED,
        TEARDOWN,
        SHUTDOWN,
        CLOSED
    };

    class conn_base : public object_base {

        virtual bool
        do_accept(const aug_endpoint& ep) = 0;

        virtual void
        do_append(aug::mplexer& mplexer, const aug_var& var) = 0;

        virtual void
        do_append(aug::mplexer& mplexer, const void* buf, size_t size) = 0;

        virtual void
        do_connected(const aug_endpoint& ep) = 0;

        virtual bool
        do_process(aug::mplexer& mplexer) = 0;

        virtual void
        do_shutdown() = 0;

        virtual void
        do_teardown() = 0;

        virtual const aug::endpoint&
        do_endpoint() const = 0;

        virtual connphase
        do_phase() const = 0;

    public:
        ~conn_base() AUG_NOTHROW;

        bool
        accept(const aug_endpoint& ep)
        {
            return do_accept(ep);
        }
        void
        append(aug::mplexer& mplexer, const aug_var& var)
        {
            do_append(mplexer, var);
        }
        void
        append(aug::mplexer& mplexer, const void* buf, size_t size)
        {
            do_append(mplexer, buf, size);
        }
        void
        connected(const aug_endpoint& ep)
        {
            do_connected(ep);
        }
        bool
        process(aug::mplexer& mplexer)
        {
            return do_process(mplexer);
        }
        void
        shutdown()
        {
            do_shutdown();
        }
        void
        teardown()
        {
            do_teardown();
        }
        const aug::endpoint&
        endpoint() const
        {
            return do_endpoint();
        }
        connphase
        phase() const
        {
            return do_phase();
        }
    };

    typedef aug::smartptr<conn_base> connptr;

    inline bool
    sendable(const conn_base& conn)
    {
        return conn.phase() < SHUTDOWN;
    }

    class established : public conn_base {

        servptr serv_;
        augas_object& sock_;
        buffer& buffer_;
        rwtimer& rwtimer_;
        aug::smartfd sfd_;
        aug::endpoint endpoint_;
        connphase phase_;
        bool close_;

        augas_object&
        do_object();

        const augas_object&
        do_object() const;

        const servptr&
        do_serv() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_append(aug::mplexer& mplexer, const aug_var& var);

        void
        do_append(aug::mplexer& mplexer, const void* buf, size_t size);

        void
        do_connected(const aug_endpoint& ep);

        bool
        do_process(aug::mplexer& mplexer);

        void
        do_shutdown();

        void
        do_teardown();

        const aug::endpoint&
        do_endpoint() const;

        connphase
        do_phase() const;

    public:
        ~established() AUG_NOTHROW;

        established(const servptr& serv, augas_object& sock, buffer& buffer,
                    rwtimer& rwtimer, const aug::smartfd& sfd,
                    const aug::endpoint& ep, bool close);
    };

    class connecting : public conn_base {

        servptr serv_;
        augas_object& sock_;
        buffer& buffer_;
        aug::connector connector_;
        aug::smartfd sfd_;
        aug::endpoint endpoint_;
        connphase phase_;

        augas_object&
        do_object();

        const augas_object&
        do_object() const;

        const servptr&
        do_serv() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_append(aug::mplexer& mplexer, const aug_var& var);

        void
        do_append(aug::mplexer& mplexer, const void* buf, size_t size);

        void
        do_connected(const aug_endpoint& ep);

        bool
        do_process(aug::mplexer& mplexer);

        void
        do_shutdown();

        void
        do_teardown();

        const aug::endpoint&
        do_endpoint() const;

        connphase
        do_phase() const;

    public:
        ~connecting() AUG_NOTHROW;

        connecting(const servptr& serv, augas_object& sock, buffer& buffer,
                   const char* host, const char* port);
    };
}

#endif // AUGAS_CONN_HPP
