/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MAUD_BUILD
#include "maud.h"

static void
stop_(void)
{
    maud_writelog(MAUD_LOGINFO, "stop_()");
}

static int
start_(struct maud_session* session)
{
    maud_writelog(MAUD_LOGINFO, "start_()");
    return 0;
}

static void
reconf_(void)
{
    maud_writelog(MAUD_LOGINFO, "reconf_()");
}

static void
event_(const char* from, const char* type, const void* user, size_t size)
{
    maud_writelog(MAUD_LOGINFO, "event_()");
}

static void
closed_(const struct maud_object* sock)
{
    maud_writelog(MAUD_LOGINFO, "closed_()");
}

static void
teardown_(const struct maud_object* sock)
{
    maud_writelog(MAUD_LOGINFO, "teardown_()");
    maud_shutdown(sock->id_, 0);
}

static int
accepted_(struct maud_object* sock, const char* addr, unsigned short port)
{
    maud_writelog(MAUD_LOGINFO, "accepted_()");
    return 0;
}

static void
connected_(struct maud_object* sock, const char* addr, unsigned short port)
{
    maud_writelog(MAUD_LOGINFO, "connected_()");
}

static void
data_(const struct maud_object* sock, const void* buf, size_t len)
{
    maud_writelog(MAUD_LOGINFO, "data_()");
    maud_send(sock->id_, buf, len);
}

static void
rdexpire_(const struct maud_object* sock, unsigned* ms)
{
    maud_writelog(MAUD_LOGINFO, "rdexpire_()");
}

static void
wrexpire_(const struct maud_object* sock, unsigned* ms)
{
    maud_writelog(MAUD_LOGINFO, "wrexpire_()");
}

static void
expire_(const struct maud_object* timer, unsigned* ms)
{
    maud_writelog(MAUD_LOGINFO, "expire_()");
}

static int
authcert_(const struct maud_object* sock, const char* subject,
          const char* issuer)
{
    maud_writelog(MAUD_LOGINFO, "authcert_()");
    return 0;
}

static const struct maud_module module_ = {
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

static const struct maud_module*
init_(const char* name)
{
    maud_writelog(MAUD_LOGINFO, "init_()");
    return &module_;
}

static void
term_(void)
{
    maud_writelog(MAUD_LOGINFO, "term_()");
}

MAUD_ENTRYPOINTS(init_, term_)
