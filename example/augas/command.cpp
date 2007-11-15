/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MAUD_BUILD
#include "maudpp.hpp"

#include "augutilpp.hpp"

using namespace aug;
using namespace maud;
using namespace std;

namespace {

    struct command : basic_session {

        bool
        do_start(const char* sname)
        {
            writelog(MAUD_LOGINFO, "starting session [%s]", sname);
            const char* serv= maud::getenv("session.command.serv");
            if (!serv)
                return false;
            tcplisten("0.0.0.0", serv);
            return true;
        }

        bool
        do_accepted(object& sock, const char* addr, unsigned short port)
        {
            sock.setuser(new shellparser());
            send(sock, "$ \r\n", 4);
            setrwtimer(sock, 15000, MAUD_TIMRD);
            return true;
        }

        void
        do_closed(const object& sock)
        {
            delete sock.user<shellparser>();
        }

        void
        do_data(const object& sock, const void* buf, size_t size)
        {
            shellparser& parser(*sock.user<shellparser>());
        }

        void
        do_rdexpire(const object& sock, unsigned& ms)
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

MAUD_ENTRYPOINTS(module::init, module::term)
