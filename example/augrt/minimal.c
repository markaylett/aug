/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augrt.h"

static void
stop_(void)
{
    augrt_writelog(AUGRT_LOGINFO, "stop_()");
}

static int
start_(struct augrt_session* session)
{
    augrt_writelog(AUGRT_LOGINFO, "start_()");
    return 0;
}

static void
reconf_(void)
{
    augrt_writelog(AUGRT_LOGINFO, "reconf_()");
}

static void
event_(const char* from, const char* type, const void* user, size_t size)
{
    augrt_writelog(AUGRT_LOGINFO, "event_()");
}

static void
closed_(const struct augrt_object* sock)
{
    augrt_writelog(AUGRT_LOGINFO, "closed_()");
}

static void
teardown_(const struct augrt_object* sock)
{
    augrt_writelog(AUGRT_LOGINFO, "teardown_()");
    augrt_shutdown(sock->id_);
}

static int
accepted_(struct augrt_object* sock, const char* addr, unsigned short port)
{
    augrt_writelog(AUGRT_LOGINFO, "accepted_()");
    return 0;
}

static void
connected_(struct augrt_object* sock, const char* addr, unsigned short port)
{
    augrt_writelog(AUGRT_LOGINFO, "connected_()");
}

static void
data_(const struct augrt_object* sock, const void* buf, size_t len)
{
    augrt_writelog(AUGRT_LOGINFO, "data_()");
    augrt_send(sock->id_, buf, len);
}

static void
rdexpire_(const struct augrt_object* sock, unsigned* ms)
{
    augrt_writelog(AUGRT_LOGINFO, "rdexpire_()");
}

static void
wrexpire_(const struct augrt_object* sock, unsigned* ms)
{
    augrt_writelog(AUGRT_LOGINFO, "wrexpire_()");
}

static void
expire_(const struct augrt_object* timer, unsigned* ms)
{
    augrt_writelog(AUGRT_LOGINFO, "expire_()");
}

static int
authcert_(const struct augrt_object* sock, const char* subject,
          const char* issuer)
{
    augrt_writelog(AUGRT_LOGINFO, "authcert_()");
    return 0;
}

static const struct augrt_module module_ = {
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

static const struct augrt_module*
init_(const char* name)
{
    augrt_writelog(AUGRT_LOGINFO, "init_()");
    return &module_;
}

static void
term_(void)
{
    augrt_writelog(AUGRT_LOGINFO, "term_()");
}

AUGRT_MODULE(init_, term_)
