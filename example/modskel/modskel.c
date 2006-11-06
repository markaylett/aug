#define AUGAS_SHARED
#define AUGAS_BUILD
#include "augas/module.h"

#include <stdlib.h>

static const char* modname_ = NULL;
static const struct augas_host* host_ = NULL;

static void
close_(const struct augas_session* s)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "close_()");
}

static int
open_(struct augas_session* s, const char* serv, const char* peer)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "open_()");
    return 0;
}

static int
data_(const struct augas_session* s, const char* buf, size_t size)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "data_()");
    host_->send_(s->sid_, buf, size, AUGAS_SESSELF);
    return 0;
}

static int
rdexpire_(const struct augas_session* s, unsigned* ms)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "rdexpire_()");
    return 0;
}

static int
wrexpire_(const struct augas_session* s, unsigned* ms)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "wrexpire_()");
    return 0;
}

static int
stop_(const struct augas_session* s)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "stop_()");
    host_->shutdown_(s->sid_);
    return 0;
}

static int
event_(int type, void* arg)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "event_()");
    return 0;
}

static int
expire_(void* arg, unsigned id, unsigned* ms)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "expire_()");
    return 0;
}

static int
reconf_(void)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "reconf_()");
    return 0;
}

static const struct augas_module fntable_ = {
    close_,
    open_,
    data_,
    rdexpire_,
    wrexpire_,
    stop_,
    event_,
    expire_,
    reconf_
};

static const struct augas_module*
load_(const char* modname, const struct augas_host* host)
{
    modname_ = modname;
    host_ = host;
    host_->writelog_(modname_, AUGAS_LOGINFO, "load_()");
    return &fntable_;
}

static void
unload_(void)
{
    host_->writelog_(modname_, AUGAS_LOGINFO, "unload_()");
    host_ = 0;
}

AUGAS_MODULE(load_, unload_)
