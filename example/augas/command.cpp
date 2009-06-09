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

    struct tostring : unary_function<pair<string, string>, string> {
        result_type
        operator ()(const argument_type& p)
        {
            string s(p.first);
            if (!p.second.empty()) {
                s += '=';
                s += p.second;
            }
            return s;
        }
    };

    string
    join(shellparser& parser)
    {
        shellpairs pairs;
        parser.reset(pairs);

        vector<string> words;
        transform(pairs.begin(), pairs.end(), back_inserter(words),
                  tostring());

        stringstream ss;
        copy(words.begin(), words.end(),
             ostream_iterator<string>(ss, "]["));

        string s(ss.str());
        // Insert before erase for empty strings.
        s.insert(0, "[");
        return s.erase(s.size() - 1);
    }

    class parser : public boxptr_base<parser>, public mpool_ops {
        shellparser impl_;
        explicit
        parser(mpoolref mpool, bool pairs)
            : impl_(mpool, pairs)
        {
        }
    public:
        ~parser() MOD_NOTHROW
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

    class command : public basic_session<command>, public mpool_ops {
        const string sname_;
        explicit
        command(const string& sname)
            : sname_(sname)
        {
        }
    public:
        ~command() MOD_NOTHROW
        {
            // Deleted from base.
        }
        mod_bool
        start()
        {
            writelog(MOD_LOGINFO, "starting session [%s]", sname_.c_str());
            const char* serv= mod::getenv("session.command.serv");
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

            send(sock, "HELLO\r\n", 7);
            setrwtimer(sock, 15000, MOD_TIMRD);
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

            for (size_t i(0); i < len; ++i)
                if (parser->append(ptr[i])) {
                    string s(join(*parser));
                    s += "\r\n";
                    send(sock, s.c_str(), s.size());
                }
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
            return 0 == strcmp(sname, "command")
                ? attach(new (tlx) command(sname))
                : null;
        }
    };

    typedef basic_module<basic_factory<command> > module;
}

MOD_ENTRYPOINTS(module::init, module::term, module::create)
