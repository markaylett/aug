#define AUGAS_SHARED
#define AUGAS_BUILD
#include "augas/module.h"

#include <stdlib.h>

static const struct augas_service* service_ = NULL;

static void
close_(const struct augas_session* s)
{
    service_->writelog_(AUGAS_LOGINFO, "close_()");
}

static int
open_(struct augas_session* s, const char* serv)
{
    service_->writelog_(AUGAS_LOGINFO, "open_()");
    return 0;
}

static int
data_(const struct augas_session* s, const char* buf, size_t size)
{
    service_->writelog_(AUGAS_LOGINFO, "data_()");
    service_->send_(s->sid_, buf, size, AUGAS_SESSELF);
    return 0;
}

static int
rdexpire_(const struct augas_session* s, unsigned* ms)
{
    service_->writelog_(AUGAS_LOGINFO, "rdexpire_()");
    return 0;
}

static int
wrexpire_(const struct augas_session* s, unsigned* ms)
{
    service_->writelog_(AUGAS_LOGINFO, "wrexpire_()");
    return 0;
}

static int
stop_(const struct augas_session* s)
{
    service_->writelog_(AUGAS_LOGINFO, "stop_()");
    service_->shutdown_(s->sid_);
    return 0;
}

static int
event_(int type, void* arg)
{
    service_->writelog_(AUGAS_LOGINFO, "event_()");
    return 0;
}

static int
expire_(void* arg, unsigned id, unsigned* ms)
{
    service_->writelog_(AUGAS_LOGINFO, "expire_()");
    return 0;
}

static int
reconf_(void)
{
    service_->writelog_(AUGAS_LOGINFO, "reconf_()");
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
load_(const struct augas_service* service)
{
    service_ = service;
    service_->writelog_(AUGAS_LOGINFO, "load_()");
    return &fntable_;
}

static void
unload_(void)
{
    service_->writelog_(AUGAS_LOGINFO, "unload_()");
    service_ = 0;
}

AUGAS_MODULE(load_, unload_)
