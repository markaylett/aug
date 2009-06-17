/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define MOD_BUILD
#include "augmodpp.hpp"

#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include "augext/blob.h"

#include <fstream>
#include <memory> // auto_ptr<>

using namespace mod;
using namespace std;

namespace {

    const char MSG[] = "hello, world!\r\n";

    struct helloblob : aug::mpool_ops {
        const char*
        getblobtype_()
        {
            return "application/octet-stream";
        }
        static const void*
        getblobdata_(size_t& size) AUG_NOTHROW
        {
            size = getblobsize_();
            return MSG;
        }
        static size_t
        getblobsize_() AUG_NOTHROW
        {
            return sizeof(MSG) - 1;
        }
    };

    aug::scoped_blob_wrapper<helloblob> hello;

    void
    dosend(const mod_handle& sock)
    {
        send(sock, MSG, sizeof(MSG) - 1);
    }

    void
    dosendv(const mod_handle& sock)
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

    class statebox : public aug::boxptr_base<statebox>,
                     public aug::mpool_ops {
        state state_;
        explicit
        statebox(int echos)
            : state_(echos)
        {
        }
    public:
        ~statebox() AUG_NOTHROW
        {
            // Deleted from base.
        }
        void*
        unboxptr_() AUG_NOTHROW
        {
            return &state_;
        }
        static aug::boxptrptr
        create(int echos)
        {
            return attach(new (aug::tlx) statebox(echos));
        }
    };

    struct eachline : aug::mpool_ops {
        aug::hires* hires_;
        vector<double>& secs_;
        void (*fn_)(const mod_handle&);
        const mod_handle& sock_;
        explicit
        eachline(aug::hires& hires, vector<double>& secs,
                 void (*fn)(const mod_handle&), const mod_handle& sock)
            : hires_(&hires),
              secs_(secs),
              fn_(fn),
              sock_(sock)
        {
        }
        void
        operator ()(std::string& tok)
        {
            state* s(aug::obtop<state>(sock_.ob_));
            secs_.push_back(elapsed(*hires_));
            if (0 == --s->torecv_)
                shutdown(sock_, 0);
            else if (0 < s->tosend_--)
                fn_(sock_);
        }
    };

    class bench : public basic_session<bench>, public aug::mpool_ops {
        const string sname_;
        void (*send_)(const mod_handle&);
        unsigned conns_, estab_, echos_;
        size_t bytes_;
        aug::hires hires_;
        map<double, double> xy_;
        explicit
        bench(const string& sname)
            : sname_(sname),
              conns_(0),
              estab_(0),
              bytes_(0),
              hires_(aug::getmpool(aug_tlx))
        {
        }
    public:
        ~bench() AUG_NOTHROW
        {
            // Deleted from base.
        }
        mod_bool
        start()
        {
            writelog(MOD_LOGINFO, "starting...");

            if (atoi(mod::getenv("session.bench.sendv", "1"))) {
                send_ = dosendv;
                writelog(MOD_LOGINFO, "sendv: yes");
            } else {
                send_ = dosend;
                writelog(MOD_LOGINFO, "sendv: no");
            }

            const char* serv = mod::getenv("session.bench.serv");
            if (!serv)
                return MOD_FALSE;

            const char* host = mod::getenv("session.bench.host",
                                           "localhost");
            const char* sslctx = mod::getenv("session.bench.sslcontext", 0);

            conns_ = atoi(mod::getenv("session.bench.conns", "100"));
            echos_ = atoi(mod::getenv("session.bench.echos", "1000"));

            writelog(MOD_LOGINFO, "host: %s", host);
            writelog(MOD_LOGINFO, "serv: %s", serv);
            if (sslctx)
                writelog(MOD_LOGINFO, "sslcontext: %s", sslctx);
            writelog(MOD_LOGINFO, "conns: %d", conns_);
            writelog(MOD_LOGINFO, "echos: %d", echos_);

            for (; estab_ < conns_; ++estab_) {
                aug::boxptrptr bp(statebox::create(echos_));
                tcpconnect(host, serv, sslctx, bp);
            }
            return MOD_TRUE;
        }
        void
        stop()
        {
        }
        void
        reconf()
        {
        }
        void
        event(const char* from, const char* type, mod_id id,
              aug::objectref ob)
        {
        }
        void
        closed(mod_handle& sock)
        {
            // Scoped retain.
            aug::objectptr ob(object_retain(aug::obptr(sock.ob_)));
            aug_assign(sock.ob_, 0);

            state* s(aug::obtop<state>(sock.ob_));
            pushxy(xy_, s->secs_);
            if (0 < --estab_) {
                writelog(MOD_LOGINFO, "%d established", estab_);
                return;
            }

            double ms(elapsed(hires_) * 1000.0);

            writelog(MOD_LOGINFO, "total time: %f ms", ms);

            ms /= static_cast<double>(conns_);
            writelog(MOD_LOGINFO, "time per conn: %f ms", ms);

            ms /= static_cast<double>(echos_);
            writelog(MOD_LOGINFO, "echos per sec: %f", 1000.0 / ms);

            double k(static_cast<double>(bytes_) / 1024.00);
            writelog(MOD_LOGINFO, "total size: %f k", k);

            stopall();

            writexy(xy_);
        }
        void
        teardown(mod_handle& sock)
        {
            mod::shutdown(sock, 0);
        }
        mod_bool
        accepted(mod_handle& sock, const char* name)
        {
            return MOD_TRUE;
        }
        void
        connected(mod_handle& sock, const char* name)
        {
            state* s(aug::obtop<state>(sock.ob_));
            s->secs_.push_back(elapsed(hires_));
            send_(sock);
            --s->tosend_;
        }
        mod_bool
        auth(mod_handle& sock, const char* subject, const char* issuer)
        {
            writelog(MOD_LOGINFO, "checking subject...");
            return MOD_TRUE;
        }
        void
        recv(mod_handle& sock, const void* buf, size_t len)
        {
            bytes_ += len;
            state* s(aug::obtop<state>(sock.ob_));
            tokenise(static_cast<const char*>(buf),
                     static_cast<const char*>(buf) + len, s->tok_, '\n',
                     eachline(hires_, s->secs_, send_, sock));
        }
        void
        error(mod_handle& sock, const char* desc)
        {
            writelog(MOD_LOGERROR, "client error: %s", desc);
        }
        void
        rdexpire(mod_handle& sock, unsigned& ms)
        {
        }
        void
        wrexpire(mod_handle& sock, unsigned& ms)
        {
        }
        void
        expire(mod_handle& timer, unsigned& ms)
        {
        }
        static sessionptr
        create(const char* sname)
        {
            return attach(new (aug::tlx) bench(sname));
        }
    };

    typedef basic_module<basic_factory<bench> > module;
}

MOD_ENTRYPOINTS(module::init, module::term, module::create)
