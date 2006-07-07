/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

static const char rcsid[] = "$Id:$";

#undef __STRICT_ANSI__ /* snprintf() */
#include "message.h"
#include "state.h"

#include "augsrv.h"
#include "augnet.h"
#include "augutil.h"
#include "augsys.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#if defined(_WIN32)
# define snprintf _snprintf
#endif /* _WIN32 */

#define PORT_ 8080

static const char* program_;

static int daemon_ = 0;
static int quit_ = 0;

static char conffile_[AUG_PATH_MAX + 1] = "";
static char rundir_[AUG_PATH_MAX + 1];
static char pidfile_[AUG_PATH_MAX + 1] = "httpd.pid";
static char logfile_[AUG_PATH_MAX + 1] = "httpd.log";
static char address_[AUG_PATH_MAX + 1] = "127.0.0.1:8080";

static const struct aug_service* service_ = NULL;
static aug_mplexer_t mplexer_ = NULL;
static int fd_ = -1;
static struct aug_conns conns_ = AUG_HEAD_INITIALIZER(conns_);
static struct aug_timers timers_ = AUG_HEAD_INITIALIZER(timers_);

static int
request_(const struct aug_var* arg, const char* initial, aug_mar_t mar,
         struct aug_messages* messages)
{
    char buf[64];
    time_t t;
    struct aug_field f;
    aug_strbuf_t s = aug_createstrbuf(0);
    struct aug_message* message;
    aug_setstrbufs(&s, "HTTP/1.0 200 OK");

    mar = aug_createmar();

    time(&t);
    strftime(buf, sizeof(buf), "%a, %d-%b-%Y %X GMT", gmtime(&t));
    f.name_ = "Date";
    f.value_ = buf;
    f.size_ = strlen(buf);
    aug_setfield(mar, &f, NULL);

    f.name_ = "Content-Type";
    f.value_ = "text/html";
    f.size_ = 9;
    aug_setfield(mar, &f, NULL);

#define MSG "<html><body><h1>Test Message</h1></body></html>"

    snprintf(buf, sizeof(buf), "%d", strlen(MSG));
    f.name_ = "Content-Length";
    f.value_ = buf;
    f.size_ = strlen(buf);
    aug_setfield(mar, &f, NULL);

    aug_setcontent(mar, MSG, strlen(MSG));

    message = aug_createmessage(s, mar);
    AUG_INSERT_TAIL(messages, message);
    return 0;
}

static void*
createconn_(aug_mplexer_t mplexer, int fd)
{
    aug_state_t state = aug_createstate(request_, NULL);
    if (!state)
        return NULL;

    if (-1 == aug_setioeventmask(mplexer, fd, AUG_IOEVENTRD)) {
        aug_freestate(state);
        return NULL;
    }

    return state;
}

static int
freeconn_(aug_state_t state, aug_mplexer_t mplexer, int fd)
{
    aug_setioeventmask(mplexer, fd, 0);
    aug_freestate(state);
    return 0;
}

static int
conn_(const struct aug_var* arg, int fd, struct aug_conns* conns)
{
    ssize_t ret;
    aug_state_t state = (aug_state_t)arg;
    int events = aug_ioevents(mplexer_, fd);

    AUG_DEBUG("checking connection '%d'", fd);

    if (events & AUG_IOEVENTRD) {

        AUG_DEBUG("handling read event");

        if (-1 == (ret = aug_readsome(state, fd))) {
            aug_perrinfo("aug_readsome() failed");
            goto fail;
        }

        if (0 == ret) {
            aug_info("closing connection '%d'", fd);
            goto fail;
        }

        if (aug_pending(state))
            aug_setioeventmask(mplexer_, fd, AUG_IOEVENTRD | AUG_IOEVENTWR);
    }

    if (events & AUG_IOEVENTWR) {

        AUG_DEBUG("handling write event");

        if (-1 == (ret = aug_writesome(state, fd))) {
            aug_perrinfo("aug_writesome() failed");
            goto fail;
        }

        if (!aug_pending(state)) {
            aug_setioeventmask(mplexer_, fd, AUG_IOEVENTRD);

            /* For HTTP test. */
            goto fail;
        }
    }

    return 1;

 fail:
    freeconn_(aug_getvarp(arg), mplexer_, fd);
    aug_close(fd);
    return 0;
}

static int
listener_(const struct aug_var* arg, int fd, struct aug_conns* conns)
{
    struct sockaddr_in addr;
    socklen_t len;
    int conn;

    AUG_DEBUG("checking listener '%d'", fd);

    if (!aug_ioevents(mplexer_, fd))
        return 1;

    AUG_DEBUG("accepting new connection");

    len = sizeof(addr);
    if (-1 == (conn = aug_accept(fd, (struct sockaddr*)&addr, &len))) {
        aug_perrinfo("aug_accept() failed");
        return 1;
    }

    aug_info("initialising new connection '%d'", conn);

    if (-1 == aug_setnonblock(conn, 1)) {
        aug_perrinfo("aug_setnonblock() failed");
        aug_close(conn);
        return 1;
    }

    if (!(arg = createconn_(mplexer_, conn))) {
        aug_perrinfo("failed to create connection");
        aug_close(conn);
        return 1;
    }

    aug_insertconn(conns, conn, conn_, arg);
    return 1;
}

static int
setconfopt_(const struct aug_var* arg, const char* name, const char* value)
{
    if (0 == aug_strcasecmp(name, "address")) {

        aug_strlcpy(address_, value, sizeof(address_));

    } else if (0 == aug_strcasecmp(name, "loglevel")) {

        unsigned int level;
        if (-1 == aug_strtoui(&level, value, 10))
            return -1;

        aug_info("setting log level: %d", level);
        aug_setloglevel(level);

    } else if (0 == aug_strcasecmp(name, "logfile")) {

        aug_strlcpy(logfile_, value, sizeof(logfile_));

    } else if (0 == aug_strcasecmp(name, "pidfile")) {

        aug_strlcpy(pidfile_, value, sizeof(pidfile_));

    } else if (0 == aug_strcasecmp(name, "rundir")) {

        if (!aug_realpath(rundir_, value, sizeof(rundir_)))
            return -1;

    } else {

        errno = EINVAL;
        return -1;
    }

    return 0;
}

static const char*
getopt_(const struct aug_var* arg, enum aug_option opt)
{
    switch (opt) {
    case AUG_OPTADMIN:
        return "Mark Aylett <mark@emantic.co.uk>";
    case AUG_OPTCONFFILE:
        return *conffile_ ? conffile_ : NULL;
    case AUG_OPTLONGNAME:
        return "HTTP Daemon";
    case AUG_OPTPIDFILE:
        return pidfile_;
    case AUG_OPTPROGRAM:
        return program_;
    case AUG_OPTSHORTNAME:
        return "httpd";
    }
    return NULL;
}

static int
reconfig_(void)
{
    if (*conffile_) {

        aug_info("reading: %s", conffile_);

        if (-1 == aug_readconf(conffile_, setconfopt_, NULL))
            return -1;
    }

    AUG_PERROR(chdir(rundir_), "chdir() failed");

    if (daemon_)
        AUG_PERRINFO(aug_openlog(logfile_), "aug_openlog() failed");

    aug_info("run directory: %s", rundir_);
    aug_info("pid file: %s", pidfile_);
    aug_info("log file: %s", logfile_);
    aug_info("log level: %d", aug_loglevel());
    aug_info("address: %s", address_);
    return 0;
}

static int
config_(const struct aug_var* arg, const char* conffile, int daemon)
{
    if (conffile && !aug_realpath(conffile_, conffile, sizeof(conffile_)))
        return -1;

    daemon_ = daemon;
    return reconfig_();
}

static int
readevent_(const struct aug_var* arg, int fd, struct aug_conns* conns)
{
    struct aug_event event;

    AUG_DEBUG("checking event pipe '%d'", fd);

    if (!aug_ioevents(mplexer_, fd))
        return 1;

    AUG_DEBUG("reading event");
    if (!aug_readevent(aug_eventin(), &event))
        aug_perrinfo("aug_readevent() failed");

    switch (event.type_) {
    case AUG_EVENTRECONF:
        aug_info("received AUG_EVENTRECONF");
        AUG_PERRINFO(reconfig_(), "failed to re-configure daemon");
        break;
    case AUG_EVENTSTATUS:
        aug_info("received AUG_EVENTSTATUS");
        break;
    case AUG_EVENTSTOP:
        aug_info("received AUG_EVENTSTOP");
        quit_ = 1;
        break;
    }
    return 1;
}

static int
init_(const struct aug_var* arg)
{
    struct sockaddr_in addr;
    struct aug_var ptr;

    aug_info("initialising daemon process");

    if (-1 == aug_setsrvlogger("aug") || !(mplexer_ = aug_createmplexer()))
        return -1;

    if (!aug_parseinet(&addr, address_))
        goto fail1;

    if (-1 == (fd_ = aug_tcplisten(&addr)))
        goto fail1;

    if (-1 == aug_insertconn(&conns_, fd_, listener_,
                             aug_setvarp(&ptr, &mplexer_)))
        goto fail2;

    if (-1 == aug_insertconn(&conns_, aug_eventin(), readevent_, &ptr)
        || -1 == aug_setioeventmask(mplexer_, fd_, AUG_IOEVENTRD)
        || -1 == aug_setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD))
        goto fail3;

    return 0;

 fail3:
    aug_freeconns(&conns_);
 fail2:
    aug_close(fd_);
 fail1:
    aug_freemplexer(mplexer_);
    return -1;
}

static int
run_(const struct aug_var* arg)
{
    struct timeval timeout;

    aug_info("running daemon process");

    while (!AUG_EMPTY(&conns_) && !quit_) {

        if (!AUG_EMPTY(&timers_))
            AUG_PERRINFO(aug_processtimers(&timers_, 0, &timeout),
                         "aug_processtimers() failed");

        /* If SA_RESTART has been set for an interrupting signal, it is
           implementation dependant whether select/poll restart or return with
           EINTR set. */

        while (-1 == aug_waitioevents(mplexer_, !AUG_EMPTY(&timers_)
                                      ? &timeout : NULL))
            aug_perrinfo("aug_waitevents() failed");

        AUG_PERRINFO(aug_processconns(&conns_),
                     "aug_processconns() failed");
    }

    aug_info("stopping daemon process");

    AUG_PERRINFO(aug_freetimers(&timers_), "aug_freetimers() failed");
    AUG_PERRINFO(aug_freeconns(&conns_), "aug_freeconns() failed");
    AUG_PERRINFO(aug_close(fd_), "aug_close() failed");
    AUG_PERRINFO(aug_freemplexer(mplexer_), "aug_freemplexer() failed");
    return 0;
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    struct aug_service service = {
        getopt_,
        config_,
        init_,
        run_,
        AUG_VARNULL
    };

    program_ = argv[0];
    aug_atexitinit(&errinfo);

    service_ = &service;
    if (!getcwd(rundir_, sizeof(rundir_))) {
        aug_perror("getcwd() failed");
        return 1;
    }

    aug_main(&service, argc, argv);
    return 1;
}
