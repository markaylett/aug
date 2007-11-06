/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMOD_BUILD
#include "augmod.h"

static void
stop_(void)
{
    augmod_writelog(AUGMOD_LOGINFO, "stop_()");
}

static int
start_(struct augmod_session* session)
{
    augmod_writelog(AUGMOD_LOGINFO, "start_()");
    return 0;
}

static void
reconf_(void)
{
    augmod_writelog(AUGMOD_LOGINFO, "reconf_()");
}

static void
event_(const char* from, const char* type, const void* user, size_t size)
{
    augmod_writelog(AUGMOD_LOGINFO, "event_()");
}

static void
closed_(const struct augmod_object* sock)
{
    augmod_writelog(AUGMOD_LOGINFO, "closed_()");
}

static void
teardown_(const struct augmod_object* sock)
{
    augmod_writelog(AUGMOD_LOGINFO, "teardown_()");
    augmod_shutdown(sock->id_, 0);
}

static int
accepted_(struct augmod_object* sock, const char* addr, unsigned short port)
{
    augmod_writelog(AUGMOD_LOGINFO, "accepted_()");
    return 0;
}

static void
connected_(struct augmod_object* sock, const char* addr, unsigned short port)
{
    augmod_writelog(AUGMOD_LOGINFO, "connected_()");
}

static void
data_(const struct augmod_object* sock, const void* buf, size_t len)
{
    augmod_writelog(AUGMOD_LOGINFO, "data_()");
    augmod_send(sock->id_, buf, len);
}

static void
rdexpire_(const struct augmod_object* sock, unsigned* ms)
{
    augmod_writelog(AUGMOD_LOGINFO, "rdexpire_()");
}

static void
wrexpire_(const struct augmod_object* sock, unsigned* ms)
{
    augmod_writelog(AUGMOD_LOGINFO, "wrexpire_()");
}

static void
expire_(const struct augmod_object* timer, unsigned* ms)
{
    augmod_writelog(AUGMOD_LOGINFO, "expire_()");
}

static int
authcert_(const struct augmod_object* sock, const char* subject,
          const char* issuer)
{
    augmod_writelog(AUGMOD_LOGINFO, "authcert_()");
    return 0;
}

static const struct augmod_proxy proxy_ = {
    stop_,
    start_,
    reconf_,
    event_,
    closed_,
    teardown_,
    accepted_,
    connected_,
    data_,
    rdexpire_,
    wrexpire_,
    expire_,
    authcert_
};

static const struct augmod_proxy*
init_(const char* name)
{
    augmod_writelog(AUGMOD_LOGINFO, "init_()");
    return &proxy_;
}

static void
term_(void)
{
    augmod_writelog(AUGMOD_LOGINFO, "term_()");
}

AUGMOD_ENTRYPOINTS(init_, term_)
