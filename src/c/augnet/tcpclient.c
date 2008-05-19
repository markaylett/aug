/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/tcpclient.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/inet.h"    /* aug_established() */

#include "augsys/unistd.h"  /* aug_close() */
#include "augsys/utility.h" /* aug_setnonblock() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h"  /* bzero() */

#include <assert.h>
#include <stdlib.h>         /* malloc() */

struct aug_tcpclient_ {
    struct addrinfo* res_, *save_;
    aug_sd sd_;
};

static int
getsockerr_(aug_sd sd, int* err)
{
    socklen_t len = sizeof(*err);
    if (-1 == aug_getsockopt(sd, SOL_SOCKET, SO_ERROR, err, &len))
        return -1;

    return 0;
}

AUGNET_API aug_tcpclient_t
aug_createtcpclient(const char* host, const char* serv)
{
    aug_tcpclient_t client;
    struct addrinfo hints, * res;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return NULL;

    if (!(client  = malloc(sizeof(struct aug_tcpclient_)))) {
        aug_destroyaddrinfo(res);
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    client->res_ = client->save_ = res;
    client->sd_ = AUG_BADSD;
    return client;
}

AUGNET_API int
aug_destroytcpclient(aug_tcpclient_t client)
{
    if (AUG_BADSD != client->sd_)
        aug_sclose(client->sd_);

    aug_destroyaddrinfo(client->save_);
    free(client);
    return 0;
}

AUGNET_API aug_sd
aug_tryconnect(aug_tcpclient_t client, struct aug_endpoint* ep, int* est)
{
    /* FIXME: needs review against assumptions in object.c. */

    aug_sd sd = client->sd_;
    client->sd_ = AUG_BADSD;

    /* Handle case where user has called after all connection attempts have
       failed. */

    if (!client->res_) {
        assert(AUG_BADSD == sd);
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("address-list exhausted"));
        return AUG_BADSD;
    }

    /* Not on initial attempt. */

    if (AUG_BADSD != sd) {

        /* Check status of pending connection. */

        switch (aug_established(sd)) {
        case -1:

            /* Error testing for establishment. */

            aug_sclose(sd);
            return AUG_BADSD;

        case 0:

            /* Established. */

            aug_getendpoint(client->res_, ep);
            goto done;

        case AUG_FAILNONE:

            /* Not established. */

            if ((client->res_ = client->res_->ai_next)) {

                /* Try next address. */

                if (-1 == aug_sclose(sd))
                    return AUG_BADSD;
                break;
            }

            /* No more addresses: set error to connection failure reason. */

            {
                int err = ECONNREFUSED;
                getsockerr_(sd, &err);
                aug_sclose(sd); /* May set errno */
                aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, err);
            }
            return AUG_BADSD;

        default:
            assert(!"unexpected return from aug_established()");
        }
    }

    /* Previous socket closed at this point. */

    do {

        /* Create socket for next address. */

        sd = aug_socket(client->res_->ai_family, client->res_->ai_socktype,
                        client->res_->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Ignore this one. */

        if (-1 == aug_ssetnonblock(sd, AUG_TRUE)) {
            aug_sclose(sd);
            return AUG_BADSD;
        }

        aug_getendpoint(client->res_, ep);

        if (0 == aug_connect(sd, ep)) {

            /* Immediate establishment. */

            goto done;
        }

        if (EINPROGRESS == aug_errno() || EWOULDBLOCK == aug_errno()) {

            /* Connection pending. */

            *est = 0;
            return client->sd_ = sd;
        }

        /* Failed for other reason. */

        if (-1 == aug_sclose(sd)) /* Ignore this one. */
            return -1;

    } while ((client->res_ = client->res_->ai_next));

    /* Error set from last aug_connect() attempt. */

    return -1;

 done:

    *est = 1;
    return sd;
}
