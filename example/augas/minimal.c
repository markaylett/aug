/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MOD_BUILD
#include "augmod.h"

static void
stop_(void)
{
    mod_writelog(MOD_LOGINFO, "stop_()");
}

static int
start_(struct mod_session* session)
{
    mod_writelog(MOD_LOGINFO, "start_()");
    return 0;
}

static void
reconf_(void)
{
    mod_writelog(MOD_LOGINFO, "reconf_()");
}

static void
event_(const char* from, const char* type, struct aug_object_* ob)
{
    mod_writelog(MOD_LOGINFO, "event_()");
}

static void
closed_(const struct mod_handle* sock)
{
    mod_writelog(MOD_LOGINFO, "closed_()");
}

static void
teardown_(const struct mod_handle* sock)
{
    mod_writelog(MOD_LOGINFO, "teardown_()");
    mod_shutdown(sock->id_, 0);
}

static int
accepted_(struct mod_handle* sock, const char* name)
{
    mod_writelog(MOD_LOGINFO, "accepted_()");
    return 0;
}

static void
connected_(struct mod_handle* sock, const char* name)
{
    mod_writelog(MOD_LOGINFO, "connected_()");
}

static void
data_(const struct mod_handle* sock, const void* buf, size_t len)
{
    mod_writelog(MOD_LOGINFO, "data_()");
    mod_send(sock->id_, buf, len);
}

static void
rdexpire_(const struct mod_handle* sock, unsigned* ms)
{
    mod_writelog(MOD_LOGINFO, "rdexpire_()");
}

static void
wrexpire_(const struct mod_handle* sock, unsigned* ms)
{
    mod_writelog(MOD_LOGINFO, "wrexpire_()");
}

static void
expire_(const struct mod_handle* timer, unsigned* ms)
{
    mod_writelog(MOD_LOGINFO, "expire_()");
}

static int
authcert_(const struct mod_handle* sock, const char* subject,
          const char* issuer)
{
    mod_writelog(MOD_LOGINFO, "authcert_()");
    return 0;
}

static const struct mod_module module_ = {
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

static const struct mod_module*
init_(const char* name)
{
    mod_writelog(MOD_LOGINFO, "init_()");
    return &module_;
}

static void
term_(void)
{
    mod_writelog(MOD_LOGINFO, "term_()");
}

MOD_ENTRYPOINTS(init_, term_)
