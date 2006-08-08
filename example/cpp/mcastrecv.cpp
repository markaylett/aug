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
                aug_error("usage: mcastrecv <mcast> <serv> [ifname]");
                return 1;
            }

            inetaddr in(argv[1]);
            smartfd sfd(aug::socket(family(in), SOCK_DGRAM));
            setreuseaddr(sfd, true);

            endpoint ep(inetany(family(in)), htons(atoi(argv[2])));
            aug::bind(sfd, ep);

            joinmcast(sfd, in, 4 == argc ? argv[3] : 0);

            mplexer mp;
            setioeventmask(mp, sfd, AUG_IOEVENTRD);

            for (;;) {
                while (AUG_RETINTR == waitioevents(mp))
                    ;

                char buf[1024];
                size_t size(read(sfd, buf, sizeof(buf)));
                buf[size] = '\0';
                aug_info("recv: [%s]", buf);
            }

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
