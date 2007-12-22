/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUM_BUILD
#include "aum.h"

static void
stop_(void)
{
    aum_writelog(AUM_LOGINFO, "stop_()");
}

static int
start_(struct aum_session* session)
{
    aum_writelog(AUM_LOGINFO, "start_()");
    return 0;
}

static void
reconf_(void)
{
    aum_writelog(AUM_LOGINFO, "reconf_()");
}

static void
event_(const char* from, const char* type, struct aub_object_* ob)
{
    aum_writelog(AUM_LOGINFO, "event_()");
}

static void
closed_(const struct aum_handle* sock)
{
    aum_writelog(AUM_LOGINFO, "closed_()");
}

static void
teardown_(const struct aum_handle* sock)
{
    aum_writelog(AUM_LOGINFO, "teardown_()");
    aum_shutdown(sock->id_, 0);
}

static int
accepted_(struct aum_handle* sock, const char* addr, unsigned short port)
{
    aum_writelog(AUM_LOGINFO, "accepted_()");
    return 0;
}

static void
connected_(struct aum_handle* sock, const char* addr, unsigned short port)
{
    aum_writelog(AUM_LOGINFO, "connected_()");
}

static void
data_(const struct aum_handle* sock, const void* buf, size_t len)
{
    aum_writelog(AUM_LOGINFO, "data_()");
    aum_send(sock->id_, buf, len);
}

static void
rdexpire_(const struct aum_handle* sock, unsigned* ms)
{
    aum_writelog(AUM_LOGINFO, "rdexpire_()");
}

static void
wrexpire_(const struct aum_handle* sock, unsigned* ms)
{
    aum_writelog(AUM_LOGINFO, "wrexpire_()");
}

static void
expire_(const struct aum_handle* timer, unsigned* ms)
{
    aum_writelog(AUM_LOGINFO, "expire_()");
}

static int
authcert_(const struct aum_handle* sock, const char* subject,
          const char* issuer)
{
    aum_writelog(AUM_LOGINFO, "authcert_()");
    return 0;
}

static const struct aum_module module_ = {
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

static const struct aum_module*
init_(const char* name)
{
    aum_writelog(AUM_LOGINFO, "init_()");
    return &module_;
}

static void
term_(void)
{
    aum_writelog(AUM_LOGINFO, "term_()");
}

AUM_ENTRYPOINTS(init_, term_)
