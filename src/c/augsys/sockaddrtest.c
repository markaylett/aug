/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#include <stdio.h>

#define HOST_ "127.0.0.1"
#define SERV_ "5000"

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    struct addrinfo hints, * res, * save;

    aug_atexitinit(&errinfo);

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(HOST_, SERV_, &hints, &res)) {
        aug_perrinfo(aug_tlerr, "aug_getaddrinfo() failed");
        return 1;
    }

    save = res;
    do {
        struct aug_endpoint ep;
        char buf[AUG_MAXHOSTSERVLEN + 1];

        aug_getendpoint(res, &ep);
        if (!aug_endpointntop(&ep, buf, sizeof(buf))) {
            aug_perrinfo(aug_tlerr, "aug_endpointntop() failed");
            goto fail;
        }

        if (0 != strcmp(buf, "127.0.0.1:5000")) {
            fprintf(stderr, "unexpected address [%s]\n", buf);
            goto fail;
        }

    } while ((res = res->ai_next));

    aug_destroyaddrinfo(save);
    return 0;

 fail:
    aug_destroyaddrinfo(save);
    return 1;
}
