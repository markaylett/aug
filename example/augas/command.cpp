/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMOD_BUILD
#include "augmodpp.hpp"

using namespace augmod;
using namespace std;

namespace {

    struct echoline {
        object sock_;
        explicit
        echoline(const object& sock)
            : sock_(sock)
        {
        }
        void
        operator()(string& line)
        {
            trim(line);
            transform(line.begin(), line.end(), line.begin(), ucase);
            line += "\r\n";

            send(sock_, line.c_str(), line.size());
        }
    };

    struct echosession : basic_session {
        bool
        do_start(const char* sname);

        bool
        do_accepted(object& sock, const char* addr, unsigned short port);

        void
        do_closed(const object& sock);

        void
        do_data(const object& sock, const void* buf, size_t size);

        void
        do_rdexpire(const object& sock, unsigned& ms);

        static session_base*
        create(const char* sname);
    };

    bool
    echosession::do_start(const char* sname)
    {
        writelog(AUGMOD_LOGINFO, "starting session [%s]", sname);
        const char* serv= augmod::getenv("session.echo.serv");
        if (!serv)
            return false;
        tcplisten("0.0.0.0", serv);
        return true;
    }

    bool
    echosession::do_accepted(object& sock, const char* addr,
                             unsigned short port)
    {
        sock.setuser(new string());
        send(sock, "HELLO\r\n", 7);
        setrwtimer(sock, 15000, AUGMOD_TIMRD);
        return true;
    }

    void
    echosession::do_closed(const object& sock)
    {
        delete sock.user<string>();
    }

    void
    echosession::do_data(const object& sock, const void* buf, size_t size)
    {
        string& tok(*sock.user<string>());
        tokenise(static_cast<const char*>(buf),
                 static_cast<const char*>(buf) + size, tok, '\n',
                 echoline(sock));
    }

    void
    echosession::do_rdexpire(const object& sock, unsigned& ms)
    {
        shutdown(sock, 0);
    }

    session_base*
    echosession::create(const char* sname)
    {
        return 0 == strcmp(sname, "echo") ? new echosession() : 0;
    }
}

typedef basic_module<basic_factory<echosession> > sample;
AUGMOD_ENTRYPOINTS(sample::init, sample::term)
