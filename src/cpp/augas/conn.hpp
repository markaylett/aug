/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_CONN_HPP
#define AUGAS_CONN_HPP

#include "augas/rwtimer.hpp"
#include "augas/sock.hpp"

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

    class conn_base : public sock_base {

        virtual bool
        do_accept(const aug_endpoint& ep) = 0;

        virtual void
        do_connected(const aug_endpoint& ep) = 0;

        virtual bool
        do_process(aug::mplexer& mplexer) = 0;

        virtual void
        do_putsome(aug::mplexer& mplexer, const void* buf, size_t size) = 0;

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
        putsome(aug::mplexer& mplexer, const void* buf, size_t size)
        {
            do_putsome(mplexer, buf, size);
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

        sessptr sess_;
        augas_sock& sock_;
        buffer& buffer_;
        rwtimer& rwtimer_;
        aug::smartfd sfd_;
        aug::endpoint endpoint_;
        connphase phase_;
        bool close_;

        augas_sock&
        do_sock();

        const augas_sock&
        do_sock() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_connected(const aug_endpoint& ep);

        bool
        do_process(aug::mplexer& mplexer);

        void
        do_putsome(aug::mplexer& mplexer, const void* buf, size_t size);

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

        established(const sessptr& sess, augas_sock& sock, buffer& buffer,
                    rwtimer& rwtimer, const aug::smartfd& sfd,
                    const aug::endpoint& ep, bool close);
    };

    class connecting : public conn_base {

        sessptr sess_;
        augas_sock& sock_;
        buffer& buffer_;
        aug::connector connector_;
        aug::smartfd sfd_;
        aug::endpoint endpoint_;
        connphase phase_;

        augas_sock&
        do_sock();

        const augas_sock&
        do_sock() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_connected(const aug_endpoint& ep);

        bool
        do_process(aug::mplexer& mplexer);

        void
        do_putsome(aug::mplexer& mplexer, const void* buf, size_t size);

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

        connecting(const sessptr& sess, augas_sock& sock, buffer& buffer,
                   const char* host, const char* serv);
    };
}

#endif // AUGAS_CONN_HPP
