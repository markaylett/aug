#include "augas.h"

#include <stdlib.h>

static const struct augas_host* host_ = NULL;

static void
term_(const struct augas_sess* sess)
{
    host_->writelog_(AUGAS_LOGINFO, "term_()");
}

static int
init_(struct augas_sess* sess)
{
    host_->writelog_(AUGAS_LOGINFO, "init_()");
    return 0;
}

static int
event_(const struct augas_sess* sess, int type, void* user)
{
    host_->writelog_(AUGAS_LOGINFO, "event_()");
    return 0;
}

static int
expire_(const struct augas_sess* sess, int tid, void* user,
        unsigned* ms)
{
    host_->writelog_(AUGAS_LOGINFO, "expire_()");
    return 0;
}

static int
reconf_(const struct augas_sess* sess)
{
    host_->writelog_(AUGAS_LOGINFO, "reconf_()");
    return 0;
}

static void
closed_(const struct augas_sock* sock)
{
    host_->writelog_(AUGAS_LOGINFO, "closed_()");
}

static int
accept_(struct augas_sock* sock, const char* addr, unsigned short port)
{
    host_->writelog_(AUGAS_LOGINFO, "accept_()");
    return 0;
}

static int
connected_(struct augas_sock* sock, const char* addr, unsigned short port)
{
    host_->writelog_(AUGAS_LOGINFO, "connected_()");
    return 0;
}

static int
data_(const struct augas_sock* sock, const char* buf, size_t size)
{
    host_->writelog_(AUGAS_LOGINFO, "data_()");
    host_->send_(sock->sess_->name_, sock->id_, buf, size, AUGAS_SNDSELF);
    return 0;
}

static int
rdexpire_(const struct augas_sock* sock, unsigned* ms)
{
    host_->writelog_(AUGAS_LOGINFO, "rdexpire_()");
    return 0;
}

static int
wrexpire_(const struct augas_sock* sock, unsigned* ms)
{
    host_->writelog_(AUGAS_LOGINFO, "wrexpire_()");
    return 0;
}

static int
teardown_(const struct augas_sock* sock)
{
    host_->writelog_(AUGAS_LOGINFO, "teardown_()");
    host_->shutdown_(sock->id_);
    return 0;
}

static const struct augas_module fntable_ = {
    term_,
    init_,
    event_,
    expire_,
    reconf_,
    closed_,
    accept_,
    connected_,
    data_,
    rdexpire_,
    wrexpire_,
    teardown_
};

static const struct augas_module*
load_(const char* name, const struct augas_host* host)
{
    host->writelog_(AUGAS_LOGINFO, "load_()");
    if (host_)
        return NULL;

    host_ = host;
    return &fntable_;
}

static void
unload_(void)
{
    host_->writelog_(AUGAS_LOGINFO, "unload_()");
    host_ = 0;
}

AUGAS_MODULE(load_, unload_)
