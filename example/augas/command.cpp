/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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

    struct command : basic_session {

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
            sock.setuser(new shellparser());
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
        do_data(const handle& sock, const void* buf, size_t size)
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

        static session_base*
        create(const char* sname)
        {
            return 0 == strcmp(sname, "command") ? new command() : 0;
        }
    };

    typedef basic_module<basic_factory<command> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
