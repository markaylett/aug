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

            if (argc < 3) {
                aug_error("usage: mcastsend <mcast> <serv> [ifname]");
                return 1;
            }

            inetaddr in(argv[1]);
            smartfd sfd(aug::socket(family(in), SOCK_DGRAM));
            if (4 == argc)
                setmcastif(sfd, argv[3]);

            endpoint ep(in, htons(atoi(argv[2])));
            sendto(sfd, "test", 4, 0, ep);

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
