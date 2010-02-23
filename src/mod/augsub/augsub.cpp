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

#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "augsubpp/base.hpp"

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    map<mod_id, rootptr> socks_;

    typedef shellpairs::const_iterator pair_iterator;

    rootptr
    getroot(mod_id id)
    {
        // Lazy creation.

        map<mod_id, rootptr>::iterator it(socks_.find(id));
        if (it == socks_.end())
            it = socks_.insert(make_pair(id, createroot())).first;
        return it->second;
    }

    void
    sendother(mod_id id, const pair<string, string>& p)
    {
        string s("* ");
        s += p.first;
        if (!p.second.empty()) {
            s += '=';
            s += p.second;
        }
        s += "\r\n";

        map<mod_id, rootptr>::const_iterator it(socks_.begin()),
            end(socks_.end());
        for (; it != end; ++it) {

            // All matching except this.

            if (it->first != id && it->second->ismatch(p.first))
                send(it->first, s.c_str(), s.size());
        }
    }

    void
    publish(mod_id id, pair_iterator it, pair_iterator end)
    {
        if (it == end)
            throw invalid_argument("missing arguments");
        do {
            sendother(id, *it);
        } while (++it != end);
    }

    void
    subscribe(mod_id id, pair_iterator it, pair_iterator end)
    {
        if (it == end)
            throw invalid_argument("missing arguments");
        rootptr root(getroot(id));
        do {
            root->insert(it->first);
        } while (++it != end);
    }

    void
    unsubscribe(mod_id id, pair_iterator it, pair_iterator end)
    {
        if (it == end)
            throw invalid_argument("missing arguments");
        rootptr root(getroot(id));
        do {
            root->erase(it->first);
        } while (++it != end);
    }

    // PUB
    // SUB
    // UNSUB

    string
    interpret(mod_id id, const string& cmd, pair_iterator it,
              pair_iterator end, bool& quit)
    {
        string s("+OK");
        try {
            if ("PUB" == cmd) {
                publish(id, it, end);
            } else if ("SUB" == cmd) {
                subscribe(id, it, end);
            } else if ("UNSUB" == cmd) {
                unsubscribe(id, it, end);
            } else if ("QUIT" == cmd) {
                quit = true;
                s += " goodbye";
            } else
                s.assign("-ERR invalid command: ").append(cmd);
        } catch (const exception& e) {
            s.assign("-ERR exception: ").append(e.what());
        }
        return s;
    }

    string
    interpret(mod_id id, const shellpairs& pairs, bool& quit)
    {
        assert(!pairs.empty());
        string cmd(pairs.front().first);
        transform(cmd.begin(), cmd.end(), cmd.begin(), aug::ucase);
        return interpret(id, cmd, pairs.begin() + 1, pairs.end(), quit);
    }

    class parser : public boxptr_base<parser>, public mpool_ops {
        shellparser impl_;
        explicit
        parser(mpoolref mpool, bool pairs)
            : impl_(mpool, pairs)
        {
        }
    public:
        ~parser() AUG_NOTHROW
        {
            // Deleted from base.
        }
        void*
        unboxptr_() AUG_NOTHROW
        {
            return &impl_;
        }
        static boxptrptr
        create(mpoolref mpool, bool pairs = false)
        {
            return attach(new (tlx) parser(mpool, pairs));
        }
    };

    class sub : public basic_session<sub>, public mpool_ops {
        const string sname_;
        explicit
        sub(const string& sname)
            : sname_(sname)
        {
        }
    public:
        ~sub() AUG_NOTHROW
        {
            // Deleted from base.
        }
        mod_bool
        start()
        {
            writelog(MOD_LOGINFO, "starting session [%s]", sname_.c_str());
            const char* serv= mod::getenv("session.sub.serv");
            if (!serv)
                return MOD_FALSE;
            tcplisten("0.0.0.0", serv);
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
        event(const char* from, const char* type, mod_id id, objectref ob)
        {
        }
        void
        closed(mod_handle& sock)
        {
            aug_assign(sock.ob_, 0);
            socks_.erase(sock.id_);
        }
        void
        teardown(mod_handle& sock)
        {
            mod::shutdown(sock, 0);
        }
        mod_bool
        accepted(mod_handle& sock, const char* name)
        {
            boxptrptr bp(parser::create(getmpool(aug_tlx), true));
            aug_assign(sock.ob_, bp.base());

            send(sock, "+OK hello\r\n", 11);
            setrwtimer(sock, 120000, MOD_TIMRD);
            return MOD_TRUE;
        }
        void
        connected(mod_handle& sock, const char* name)
        {
        }
        mod_bool
        auth(mod_handle& sock, const char* subject, const char* issuer)
        {
            return MOD_TRUE;
        }
        void
        recv(mod_handle& sock, const void* buf, size_t len)
        {
            shellparser* parser(obtop<shellparser>(sock.ob_));
            const char* ptr(static_cast<const char*>(buf));

            for (size_t i(0); i < len; ++i) {

                if (parser->append(ptr[i])) {

                    shellpairs pairs;
                    parser->reset(pairs);

                    if (!pairs.empty()) {

                        bool quit(false);
                        string s(interpret(sock.id_, pairs, quit));
                        s += "\r\n";
                        send(sock, s.c_str(), s.size());

                        if (quit) {
                            shutdown(sock, 0);
                            break;
                        }
                    }
                }
            }
        }
        void
        mrecv(const char* node, unsigned sess, unsigned short type,
              const void* buf, size_t len)
        {
        }
        void
        error(mod_handle& sock, const char* desc)
        {
        }
        void
        rdexpire(mod_handle& sock, unsigned& ms)
        {
            shutdown(sock, 0);
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
            return 0 == strcmp(sname, "sub")
                ? attach(new (tlx) sub(sname))
                : null;
        }
    };

    typedef basic_module<basic_factory<sub> > module;
}

MOD_ENTRYPOINTS(module::init, module::term, module::create)
