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

            if (3 != argc) {
                aug_error("usage: mcastsend <host> <serv>");
                return 1;
            }

            struct aug_inetaddr in;
            inetaton(argv[1], in);
            smartfd sfd(aug::socket(in.family_, SOCK_DGRAM, 0));
            cout << inetntoa(in) << endl;

            joinmcast(sfd, in, "eth0");

            struct aug_endpoint ep;
            getendpoint(in, ep, atoi(argv[2]));

            char tobysays[6] = "hello";
            aug::sendto(sfd, tobysays, sizeof(tobysays), 0, ep);

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
