#include "augas.h"

static void
stop_(const struct augas_serv* serv)
{
    augas_writelog(AUGAS_LOGINFO, "stop_()");
}

static int
start_(struct augas_serv* serv)
{
    augas_writelog(AUGAS_LOGINFO, "start_()");
    return 0;
}

static int
reconf_(const struct augas_serv* serv)
{
    augas_writelog(AUGAS_LOGINFO, "reconf_()");
    return 0;
}

static int
event_(const struct augas_serv* serv, const char* from,
        const struct augas_event* event)
{
    augas_writelog(AUGAS_LOGINFO, "event_()");
    return 0;
}

static void
closed_(const struct augas_object* sock)
{
    augas_writelog(AUGAS_LOGINFO, "closed_()");
}

static int
teardown_(const struct augas_object* sock)
{
    augas_writelog(AUGAS_LOGINFO, "teardown_()");
    augas_shutdown(sock->id_);
    return 0;
}

static int
accept_(struct augas_object* sock, const char* addr, unsigned short port)
{
    augas_writelog(AUGAS_LOGINFO, "accept_()");
    return 0;
}

static int
connected_(struct augas_object* sock, const char* addr, unsigned short port)
{
    augas_writelog(AUGAS_LOGINFO, "connected_()");
    return 0;
}

static int
data_(const struct augas_object* sock, const char* buf, size_t size)
{
    augas_writelog(AUGAS_LOGINFO, "data_()");
    augas_send(sock->id_, buf, size);
    return 0;
}

static int
rdexpire_(const struct augas_object* sock, unsigned* ms)
{
    augas_writelog(AUGAS_LOGINFO, "rdexpire_()");
    return 0;
}

static int
wrexpire_(const struct augas_object* sock, unsigned* ms)
{
    augas_writelog(AUGAS_LOGINFO, "wrexpire_()");
    return 0;
}

static int
expire_(const struct augas_object* timer, unsigned* ms)
{
    augas_writelog(AUGAS_LOGINFO, "expire_()");
    return 0;
}

static const struct augas_module module_ = {
    stop_,
    start_,
    reconf_,
    event_,
    closed_,
    teardown_,
    accept_,
    connected_,
    data_,
    rdexpire_,
    wrexpire_,
    expire_
};

static const struct augas_module*
init_(const char* name)
{
    augas_writelog(AUGAS_LOGINFO, "init_()");
    return &module_;
}

static void
term_(void)
{
    augas_writelog(AUGAS_LOGINFO, "term_()");
}

AUGAS_MODULE(init_, term_)
