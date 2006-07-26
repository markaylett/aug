#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

int
main(int argc, char* argv[])
{
    try {

        struct aug_errinfo errinfo;
        scoped_init init(errinfo);

        try {

            if (4 != argc) {
                aug_error("usage: mcastsend <hostserv> <mcast> <ifname>");
                return 1;
            }

            struct aug_hostserv hs;
            parsehostserv(argv[1], hs);

            struct endpoint ep;
            smartfd sfd(udpserver(hs.host_, hs.serv_, ep));

            struct inetaddr in;
            getinetaddr(ep, in);
            cout << inetntop(in) << endl;

            inetpton(argv[2], in);
            joinmcast(sfd, in, argv[3]);

        } catch (const std::exception& e) {
            aug_perrinfo(e.what());
            return 1;
        }

    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}