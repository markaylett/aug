/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp.hpp"
#include "augsyspp.hpp"

#include <list>

using namespace aug;
using namespace std;

namespace {
    class session_base {
    public:
        virtual
        ~session_base() AUG_NOTHROW
        {
        }
    };

    class tcpclient {

        session_base& session_;
        list<pair<string, string> > hostservs_;
        auto_ptr<connector> connector_;
        endpoint ep_;
        smartfd sfd_;

        tcpclient(const tcpclient&);

        tcpclient&
        operator =(const tcpclient&);

    public:
        ~tcpclient() AUG_NOTHROW
        {
        }
        tcpclient(const string& host, const string& serv,
                  session_base& session)
            : session_(session),
              ep_(null),
              sfd_(null)
        {
            hostservs_.push_back(make_pair(host, serv));
        }
        void
        addfailover(const string& host, const string& serv)
        {
            hostservs_.push_back(make_pair(host, serv));
        }
    };
}

typedef logic_error error;

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    try {

        endpoint ep(null);
        connector ctor("127.0.0.1", "10000");

        std::pair<smartfd, bool> xy(tryconnect(ctor, ep));
        if (!xy.second) {

            mplexer mp;
            setfdeventmask(mp, xy.first, AUG_FDEVENTALL);
            waitfdevents(mp);

            // Assuming that there is no endpoint, an exception should now be
            // thrown.

            try {
                xy = tryconnect(ctor, ep);
            } catch (...) {
                if (ECONNREFUSED == aug_errno())
                    return 0;
                throw;
            }
        }
        throw error("error not thrown by tryconnect()");

    } AUG_PERRINFOCATCH;
    return 1;
}
