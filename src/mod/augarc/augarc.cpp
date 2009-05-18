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

#include "augarcpp/base.hpp"

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    map<mod_id, rootptr> socks_;

    typedef deque<string>::const_iterator word_iterator;

    rootptr
    getroot(mod_id id)
    {
        map<mod_id, rootptr>::iterator it(socks_.find(id));
        if (it == socks_.end())
            it = socks_.insert(make_pair(id, createroot())).first;
        return it->second;
    }

    void
    sendall(mod_id id, const string& path)
    {
        string s("* ");
        s += path;
        s += "\r\n";

        map<mod_id, rootptr>::const_iterator it(socks_.begin()),
            end(socks_.end());
        for (; it != end; ++it) {
            if (it->first != id && it->second->ismatch(path))
                send(it->first, s.c_str(), s.size());
        }
    }

    void
    publish(mod_id id, word_iterator it, word_iterator end)
    {
        if (it == end)
            throw invalid_argument("missing arguments");
        do {
            sendall(id, *it++);
        } while (it != end);
    }

    void
    subscribe(mod_id id, word_iterator it, word_iterator end)
    {
        if (it == end)
            throw invalid_argument("missing arguments");
        rootptr root(getroot(id));
        do {
            root->insert(*it++);
        } while (it != end);
    }

    void
    unsubscribe(mod_id id, word_iterator it, word_iterator end)
    {
        if (it == end)
            throw invalid_argument("missing arguments");
        rootptr root(getroot(id));
        do {
            root->erase(*it++);
        } while (it != end);
    }

    // PUB
    // SUB
    // UNS

    string
    interpret(mod_id id, const string& cmd, word_iterator it,
              word_iterator end, bool& quit)
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
    interpret(mod_id id, const deque<string>& words, bool& quit)
    {
        assert(!words.empty());
        string cmd(words.front());
        transform(cmd.begin(), cmd.end(), cmd.begin(), aug::ucase);
        return interpret(id, cmd, words.begin() + 1, words.end(), quit);
    }

    struct arc : basic_session, mpool_ops {

        bool
        do_start(const char* sname)
        {
            writelog(MOD_LOGINFO, "starting session [%s]", sname);
            const char* serv= mod::getenv("session.arc.serv");
            if (!serv)
                return false;
            tcplisten("0.0.0.0", serv);
            return true;
        }

        bool
        do_accepted(handle& sock, const char* name)
        {
            sock.setuser(new (tlx) shellparser(getmpool(aug_tlx)));
            send(sock, "+OK hello\r\n", 11);
            setrwtimer(sock, 120000, MOD_TIMRD);
            return true;
        }

        void
        do_closed(const handle& sock)
        {
            socks_.erase(sock.id());
            delete sock.user<shellparser>();
        }

        void
        do_recv(const handle& sock, const void* buf, size_t size)
        {
            shellparser& parser(*sock.user<shellparser>());
            const char* ptr(static_cast<const char*>(buf));

            for (size_t i(0); i < size; ++i) {

                if (parser.append(ptr[i])) {

                    deque<string> words;
                    parser.reset(words);

                    if (!words.empty()) {

                        bool quit(false);
                        string s(interpret(sock.id(), words, quit));
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
        do_rdexpire(const handle& sock, unsigned& ms)
        {
            shutdown(sock, 0);
        }

        ~arc() MOD_NOTHROW
        {
        }

        static session_base*
        create(const char* sname)
        {
            return 0 == strcmp(sname, "arc") ? new (tlx) arc() : 0;
        }
    };

    typedef basic_module<basic_factory<arc> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
