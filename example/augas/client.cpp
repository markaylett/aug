/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MAUD_BUILD
#include "maudpp.hpp"

#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include "augobj/blob.h"

#include <fstream>
#include <memory> // auto_ptr<>

using namespace maud;
using namespace std;

namespace {

    const char MSG[] = "hello, world!\r\n";

    struct helloblob {
        static const void*
        blobdata_(size_t* size) AUG_NOTHROW
        {
            if (size)
                *size = blobsize_();
            return MSG;
        }
        static size_t
        blobsize_() AUG_NOTHROW
        {
            return sizeof(MSG) - 1;
        }
    };

    aug::scoped_blob<helloblob> hello;

    void
    dosend(handle& sock)
    {
        send(sock, MSG, sizeof(MSG) - 1);
    }

    void
    dosendv(handle& sock)
    {
        sendv(sock, hello.get());
    }

    void
    pushxy(map<double, double>& xy, const vector<double>& secs)
    {
        if (secs.empty())
            return;

        vector<double>::const_iterator it(secs.begin()), end(secs.end());
        double first(*it);
        for (++it; it != end; ++it) {

            double second(*it - first);
            first = *it;
            xy.insert(make_pair(first, second));
        }
    }

    void
    writexy(const map<double, double>& xy)
    {
        ofstream os("bench.dat");
        map<double, double>::const_iterator it(xy.begin()), end(xy.end());
        for (; it != end; ++it)
            os << it->first << ' ' << it->second * 1000.0 << endl;
    }

    struct state {
        string tok_;
        unsigned tosend_, torecv_;
        vector<double> secs_;
        explicit
        state(int echos)
            : tosend_(echos),
              torecv_(echos)
        {
        }
    };

    struct eachline {
        void (*fn_)(handle&);
        handle sock_;
        aug::clock* clock_;
        vector<double>& secs_;
        explicit
        eachline(void (*fn)(handle&), const handle& sock,
                 aug::clock& clock, vector<double>& secs)
            : fn_(fn),
              sock_(sock),
              clock_(&clock),
              secs_(secs)
        {
        }
        void
        operator ()(std::string& tok)
        {
            state& s(*sock_.user<state>());
            secs_.push_back(elapsed(*clock_));
            if (0 == --s.torecv_)
                shutdown(sock_, 0);
            else if (0 < s.tosend_--)
                fn_(sock_);
        }
    };

    struct bench : basic_session {
        void (*send_)(handle&);
        unsigned conns_, estab_, echos_;
        size_t bytes_;
        aug::clock clock_;
        map<double, double> xy_;
        bool
        do_start(const char* sname)
        {
            writelog(MAUD_LOGINFO, "starting...");

            if (atoi(maud::getenv("session.bench.sendv", "1"))) {
                send_ = dosendv;
                maud_writelog(MAUD_LOGINFO, "sendv: yes");
            } else {
                send_ = dosend;
                maud_writelog(MAUD_LOGINFO, "sendv: no");
            }

            const char* serv = maud::getenv("session.bench.serv");
            if (!serv)
                return false;

            const char* host = maud::getenv("session.bench.host",
                                            "localhost");
            conns_ = atoi(maud::getenv("session.bench.conns", "100"));
            echos_ = atoi(maud::getenv("session.bench.echos", "1000"));

            maud_writelog(MAUD_LOGINFO, "host: %s", host);
            maud_writelog(MAUD_LOGINFO, "serv: %s", serv);
            maud_writelog(MAUD_LOGINFO, "conns: %d", conns_);
            maud_writelog(MAUD_LOGINFO, "echos: %d", echos_);

            for (; estab_ < conns_; ++estab_)
                tcpconnect(host, serv, new state(echos_));
            return true;
        }
        void
        do_closed(const handle& sock)
        {
            auto_ptr<state> s(sock.user<state>());
            pushxy(xy_, s->secs_);
            if (0 < --estab_) {
                maud_writelog(MAUD_LOGINFO, "%d established", estab_);
                return;
            }

            double ms(elapsed(clock_) * 1000.0);

            maud_writelog(MAUD_LOGINFO, "total time: %f ms", ms);

            ms /= static_cast<double>(conns_);
            maud_writelog(MAUD_LOGINFO, "time per conn: %f ms", ms);

            ms /= static_cast<double>(echos_);
            maud_writelog(MAUD_LOGINFO, "echos per sec: %f", 1000.0 / ms);

            double k(static_cast<double>(bytes_) / 1024.00);
            maud_writelog(MAUD_LOGINFO, "total size: %f k", k);

            stopall();

            writexy(xy_);
        }
        void
        do_connected(handle& sock, const char* addr, unsigned short port)
        {
            const char* sslctx = maud::getenv("session.bench.sslcontext", 0);
            if (sslctx) {
                writelog(MAUD_LOGINFO, "sslcontext: %s", sslctx);
                setsslclient(sock, sslctx);
            }

            state& s(*sock.user<state>());
            s.secs_.push_back(elapsed(clock_));
            send_(sock);
            --s.tosend_;
        }
        void
        do_data(const handle& sock, const void* buf, size_t len)
        {
            bytes_ += len;
            state& s(*sock.user<state>());
            tokenise(static_cast<const char*>(buf),
                     static_cast<const char*>(buf) + len, s.tok_, '\n',
                     eachline(send_, sock, clock_, s.secs_));
        }
        bool
        do_authcert(const handle& sock, const char* subject,
                    const char* issuer)
        {
            maud_writelog(MAUD_LOGINFO, "checking subject...");
            return true;
        }
        ~bench() AUG_NOTHROW
        {
        }
        bench()
            : conns_(0),
              estab_(0),
              bytes_(0)
        {
        }
        static session_base*
        create(const char* sname)
        {
            return new bench();
        }
    };

    typedef basic_module<basic_factory<bench> > module;
}

MAUD_ENTRYPOINTS(module::init, module::term)
