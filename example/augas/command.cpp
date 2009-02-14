/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    class method_base {
        virtual bool
        do_call(ostream& os, const deque<string>& args) = 0;
    public:
        virtual
        ~method_base() AUG_NOTHROW
        {
        }
        bool
        call(ostream& os, const deque<string>& args)
        {
            return do_call(os, args);
        }
        bool
        operator ()(ostream& os, const deque<string>& args)
        {
            return do_call(os, args);
        }
    };

    typedef smartptr<method_base> methodptr;

    string
    join(shellparser& parser)
    {
        deque<string> words;
        stringstream ss;
        parser.reset(words);
        copy(words.begin(), words.end(),
             ostream_iterator<string>(ss, "]["));
        string s(ss.str());
        // Insert before erase for empty strings.
        s.insert(0, "[");
        return s.erase(s.size() - 1);
    }

    struct command : basic_session, mpool_ops {

        bool
        do_start(const char* sname)
        {
            writelog(MOD_LOGINFO, "starting session [%s]", sname);
            const char* serv= mod::getenv("session.command.serv");
            if (!serv)
                return false;
            tcplisten("0.0.0.0", serv);
            return true;
        }

        bool
        do_accepted(handle& sock, const char* name)
        {
            sock.setuser(new (tlx) shellparser(getmpool(aug_tlx)));
            send(sock, "HELLO\r\n", 7);
            setrwtimer(sock, 15000, MOD_TIMRD);
            return true;
        }

        void
        do_closed(const handle& sock)
        {
            delete sock.user<shellparser>();
        }

        void
        do_recv(const handle& sock, const void* buf, size_t size)
        {
            shellparser& parser(*sock.user<shellparser>());
            const char* ptr(static_cast<const char*>(buf));
            for (size_t i(0); i < size; ++i)
                if (parser.append(ptr[i])) {
                    string s(join(parser));
                    s += "\r\n";
                    send(sock, s.c_str(), s.size());
                }
        }

        void
        do_rdexpire(const handle& sock, unsigned& ms)
        {
            shutdown(sock, 0);
        }

        ~command() MOD_NOTHROW
        {
        }

        static session_base*
        create(const char* sname)
        {
            return 0 == strcmp(sname, "command") ? new (tlx) command() : 0;
        }
    };

    typedef basic_module<basic_factory<command> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
