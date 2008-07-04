/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MOD_BUILD
#include "augmodpp.hpp"

#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include "augext/blob.h"

#include <fstream>
#include <memory> // auto_ptr<>

namespace mod = aug::mod;

using namespace mod;
using namespace std;

namespace {

    const char MSG[] = "hello, world!\r\n";

    struct helloblob {
        static const void*
        getblobdata_(size_t* size) AUG_NOTHROW
        {
            if (size)
                *size = getblobsize_();
            return MSG;
        }
        static size_t
        getblobsize_() AUG_NOTHROW
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
        aug::hires* hires_;
        vector<double>& secs_;
        explicit
        eachline(void (*fn)(handle&), const handle& sock,
                 aug::hires& hires, vector<double>& secs)
            : fn_(fn),
              sock_(sock),
              hires_(&hires),
              secs_(secs)
        {
        }
        void
        operator ()(std::string& tok)
        {
            state& s(*sock_.user<state>());
            secs_.push_back(elapsed(*hires_));
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
        aug::hires hires_;
        map<double, double> xy_;
        bool
        do_start(const char* sname)
        {
            writelog(MOD_LOGINFO, "starting...");

            if (atoi(mod::getenv("session.bench.sendv", "1"))) {
                send_ = dosendv;
                mod_writelog(MOD_LOGINFO, "sendv: yes");
            } else {
                send_ = dosend;
                mod_writelog(MOD_LOGINFO, "sendv: no");
            }

            const char* serv = mod::getenv("session.bench.serv");
            if (!serv)
                return false;

            const char* host = mod::getenv("session.bench.host",
                                           "localhost");
            const char* sslctx = mod::getenv("session.bench.sslcontext", 0);

            conns_ = atoi(mod::getenv("session.bench.conns", "100"));
            echos_ = atoi(mod::getenv("session.bench.echos", "1000"));

            mod_writelog(MOD_LOGINFO, "host: %s", host);
            mod_writelog(MOD_LOGINFO, "serv: %s", serv);
            if (sslctx)
                writelog(MOD_LOGINFO, "sslcontext: %s", sslctx);
            mod_writelog(MOD_LOGINFO, "conns: %d", conns_);
            mod_writelog(MOD_LOGINFO, "echos: %d", echos_);

            for (; estab_ < conns_; ++estab_)
                tcpconnect(host, serv, sslctx, new state(echos_));
            return true;
        }
        void
        do_closed(const handle& sock)
        {
            auto_ptr<state> s(sock.user<state>());
            pushxy(xy_, s->secs_);
            if (0 < --estab_) {
                mod_writelog(MOD_LOGINFO, "%d established", estab_);
                return;
            }

            double ms(elapsed(hires_) * 1000.0);

            mod_writelog(MOD_LOGINFO, "total time: %f ms", ms);

            ms /= static_cast<double>(conns_);
            mod_writelog(MOD_LOGINFO, "time per conn: %f ms", ms);

            ms /= static_cast<double>(echos_);
            mod_writelog(MOD_LOGINFO, "echos per sec: %f", 1000.0 / ms);

            double k(static_cast<double>(bytes_) / 1024.00);
            mod_writelog(MOD_LOGINFO, "total size: %f k", k);

            stopall();

            writexy(xy_);
        }
        void
        do_connected(handle& sock, const char* name)
        {
            state& s(*sock.user<state>());
            s.secs_.push_back(elapsed(hires_));
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
                     eachline(send_, sock, hires_, s.secs_));
        }
        bool
        do_authcert(const handle& sock, const char* subject,
                    const char* issuer)
        {
            mod_writelog(MOD_LOGINFO, "checking subject...");
            return true;
        }
        ~bench() AUG_NOTHROW
        {
        }
        bench()
            : conns_(0),
              estab_(0),
              bytes_(0),
              hires_(aug::getmpool(aug_tlx))
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

MOD_ENTRYPOINTS(module::init, module::term)
