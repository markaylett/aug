/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_CONN_HPP
#define AUGAS_CONN_HPP

#include "augas/buffer.hpp"
#include "augas/file.hpp"

#include "augsyspp.hpp"
#include "augutilpp.hpp"
#include "augnetpp.hpp"

namespace augas {

    enum connphase {
        CONNECTING,
        CONNECTED,
        TEARDOWN,
        SHUTDOWN,
        CLOSED
    };

    class rwtimer_base : public aug::timercb_base {

        virtual void
        do_setrwtimer(unsigned ms, unsigned flags) = 0;

        virtual void
        do_resetrwtimer(unsigned ms, unsigned flags) = 0;

        virtual void
        do_resetrwtimer(unsigned flags) = 0;

        virtual void
        do_cancelrwtimer(unsigned flags) = 0;

    public:
        ~rwtimer_base() AUG_NOTHROW;

        void
        setrwtimer(unsigned ms, unsigned flags)
        {
            do_setrwtimer(ms, flags);
        }
        void
        resetrwtimer(unsigned ms, unsigned flags)
        {
            do_resetrwtimer(ms, flags);
        }
        void
        resetrwtimer(unsigned flags)
        {
            do_resetrwtimer(flags);
        }
        void
        cancelrwtimer(unsigned flags)
        {
            do_cancelrwtimer(flags);
        }
    };

    typedef aug::smartptr<rwtimer_base> rwtimerptr;

    class rwtimer : public rwtimer_base {

        sessptr sess_;
        const augas_file& file_;
        aug::timer rdtimer_;
        aug::timer wrtimer_;

        void
        do_callback(aug::idref ref, unsigned& ms, aug_timers& timers);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned flags);

        void
        do_cancelrwtimer(unsigned flags);

    public:
        ~rwtimer() AUG_NOTHROW;

        rwtimer(const sessptr& sess, const augas_file& file,
                aug::timers& timers);
    };

    class conn_base : public file_base {

        virtual bool
        do_accept(const aug_endpoint& ep) = 0;

        virtual void
        do_connect(const aug_endpoint& ep) = 0;

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
        connect(const aug_endpoint& ep)
        {
            do_connect(ep);
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
        connphase phase(conn.phase());
        return CONNECTED == phase || TEARDOWN == phase;
    }

    class connected : public conn_base {

        sessptr sess_;
        augas_file& file_;
        rwtimer& rwtimer_;
        aug::endpoint endpoint_;
        aug::smartfd sfd_;
        buffer buffer_;
        connphase phase_;
        bool close_;

        augas_file&
        do_file();

        const augas_file&
        do_file() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_connect(const aug_endpoint& ep);

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
        ~connected() AUG_NOTHROW;

        connected(const sessptr& sess, augas_file& file, rwtimer& rwtimer,
                  const aug::smartfd& sfd, const aug::endpoint& ep,
                  bool close);
    };

    class connecting : public conn_base {

        sessptr sess_;
        augas_file& file_;
        aug::connector connector_;
        aug::endpoint endpoint_;
        aug::smartfd sfd_;
        connphase phase_;

        augas_file&
        do_file();

        const augas_file&
        do_file() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_connect(const aug_endpoint& ep);

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

        connecting(const sessptr& sess, augas_file& file, const char* host,
                   const char* serv);
    };

    class client : public rwtimer_base, public conn_base {

        augas_file file_;
        rwtimer rwtimer_;
        connptr conn_;

        // rwtimer_base.

        void
        do_callback(aug::idref ref, unsigned& ms, aug_timers& timers);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned flags);

        void
        do_cancelrwtimer(unsigned flags);

        // conn_base.

        augas_file&
        do_file();

        const augas_file&
        do_file() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_connect(const aug_endpoint& ep);

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
        ~client() AUG_NOTHROW;

        client(const sessptr& sess, augas_id cid, void* user,
               aug::timers& timers, const char* host, const char* serv);
    };

    class server : public rwtimer_base, public conn_base {

        augas_file file_;
        rwtimer rwtimer_;
        connected conn_;

        // rwtimer_base.

        void
        do_callback(aug::idref ref, unsigned& ms, aug_timers& timers);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned flags);

        void
        do_cancelrwtimer(unsigned flags);

        // conn_base.

        augas_file&
        do_file();

        const augas_file&
        do_file() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_connect(const aug_endpoint& ep);

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
        ~server() AUG_NOTHROW;

        server(const sessptr& sess, augas_id cid, void* user,
               aug::timers& timers, const aug::smartfd& sfd,
               const aug::endpoint& ep);
    };
}

#endif // AUGAS_CONN_HPP
