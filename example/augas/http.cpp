#include "augaspp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augmarpp.hpp"

#include <sstream>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    struct handler : basic_marhandler {
        static void
        message(const aug_var& var, const char* initial, aug_mar_t mar)
        {
            augas_id id((augas_id)var.arg_);

            header header(mar);
            header::const_iterator it(header.begin()),
                end(header.end());
            for (; it != end; ++it)
                aug_info("%s: %s", *it, header.getfield(it));

            char buf[64];
            time_t t;
            time(&t);
            strftime(buf, sizeof(buf), "%a, %d-%b-%Y %X GMT", gmtime(&t));

#define MSG "<html><body><h1>Test Message</h1></body></html>"

            stringstream ss;
            ss << "HTTP/1.0 200 OK\r\n"
               << "Date: " << buf << "\r\n"
               << "Content-Type: text/html\r\n"
               << "Content-Length: " << strlen(MSG) << "\r\n"
               << "\r\n"
               << MSG;

            send(id, ss.str().c_str(), ss.str().size());
            shutdown(id);
        }
    };

    struct httpserv : basic_serv {
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");
            const char* serv = augas::getenv("service.http.serv");
            if (!serv)
                return false;

            tcplisten("0.0.0.0", serv);
            return true;
        }
        void
        do_closed(const object& sock)
        {
            auto_ptr<marparser> parser(sock.user<marparser>());
            endmar(*parser);
        }
        bool
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            aug_var var = { 0, (void*)sock.id() };
            sock.setuser(new marparser(0, marhandler<handler>(), var));
            setrwtimer(sock, 15000, AUGAS_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const void* buf, size_t size)
        {
            string s(static_cast<const char*>(buf), size);
            writelog(AUGAS_LOGINFO, "do_data() [%s]", s.c_str());
            marparser& parser(*sock.user<marparser>());
            parsemar(parser, static_cast<const char*>(buf),
                     static_cast<unsigned>(size));
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
            writelog(AUGAS_LOGINFO, "no data received for 15 seconds");
            shutdown(sock);
        }
        static serv_base*
        create(const char* sname)
        {
            return new httpserv();
        }
    };

    typedef basic_module<basic_factory<httpserv> > module;
}

AUGAS_MODULE(module::init, module::term)
