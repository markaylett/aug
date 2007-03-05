/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

static const char rcsid[] = "$Id$";

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
static char rundir_[AUG_PATH_MAX + 1] = "";
static char pidfile_[AUG_PATH_MAX + 1] = "httpd.pid";
static char logfile_[AUG_PATH_MAX + 1] = "httpd.log";
static char address_[AUG_PATH_MAX + 1] = "127.0.0.1:8080";

static const struct aug_service* service_ = NULL;
static aug_mplexer_t mplexer_ = NULL;
static int fd_ = -1;
static struct aug_files files_ = AUG_HEAD_INITIALIZER(files_);
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

    snprintf(buf, sizeof(buf), "%d", (int)strlen(MSG));
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
        aug_destroystate(state);
        return NULL;
    }

    return state;
}

static int
destroyconn_(aug_state_t state, aug_mplexer_t mplexer, int fd)
{
    aug_setioeventmask(mplexer, fd, 0);
    aug_destroystate(state);
    return 0;
}

static int
conn_(int fd, const struct aug_var* arg, struct aug_files* files)
{
    ssize_t ret;
    aug_state_t state = aug_getvarp(arg);
    int events = aug_ioevents(mplexer_, fd);

    AUG_DEBUG0("checking connection '%d'", fd);

    if (events & AUG_IOEVENTRD) {

        AUG_DEBUG0("handling read event");

        if (-1 == (ret = aug_readsome(state, fd))) {
            aug_perrinfo(NULL, "aug_readsome() failed");
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

        AUG_DEBUG0("handling write event");

        if (-1 == (ret = aug_writesome(state, fd))) {
            aug_perrinfo(NULL, "aug_writesome() failed");
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
    destroyconn_(aug_getvarp(arg), mplexer_, fd);
    aug_close(fd);
    return 0;
}

static int
listener_(int fd, const struct aug_var* arg, struct aug_files* files)
{
    struct aug_endpoint ep;
    int conn;
    struct aug_var var;

    AUG_DEBUG0("checking listener '%d'", fd);

    if (!aug_ioevents(mplexer_, fd))
        return 1;

    AUG_DEBUG0("accepting new connection");

    if (-1 == (conn = aug_accept(fd, &ep))) {
        aug_perrinfo(NULL, "aug_accept() failed");
        return 1;
    }

    aug_info("initialising new connection '%d'", conn);

    if (-1 == aug_setnonblock(conn, 1)) {
        aug_perrinfo(NULL, "aug_setnonblock() failed");
        aug_close(conn);
        return 1;
    }

    aug_setvarp(&var, createconn_(mplexer_, conn), NULL);
    if (aug_isnull(&var)) {
        aug_perrinfo(NULL, "failed to create connection");
        aug_close(conn);
        return 1;
    }

    aug_insertfile(files, conn, conn_, &var);
    return 1;
}

static int
setconfopt_(const struct aug_var* arg, const char* name, const char* value)
{
    if (0 == aug_strcasecmp(name, "address")) {

        aug_strlcpy(address_, value, sizeof(address_));

    } else if (0 == aug_strcasecmp(name, "loglevel")) {

        unsigned level;
        if (-1 == aug_strtoui(&level, value, 10))
            return -1;

        aug_info("setting log level: %d", level);
        aug_setloglevel(level);

    } else if (0 == aug_strcasecmp(name, "logfile")) {

        aug_strlcpy(logfile_, value, sizeof(logfile_));

    } else if (0 == aug_strcasecmp(name, "pidfile")) {

        aug_strlcpy(pidfile_, value, sizeof(pidfile_));

    } else if (0 == aug_strcasecmp(name, "rundir")) {

        /* Once set, the run directory should not change. */

        if (!*rundir_ && !aug_realpath(rundir_, value, sizeof(rundir_)))
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
    case AUG_OPTCONFFILE:
        return *conffile_ ? conffile_ : NULL;
    case AUG_OPTEMAIL:
        return "Mark Aylett <mark@emantic.co.uk>";
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
reconf_(void)
{
    AUG_PERROR(chdir(rundir_), "chdir() failed");

    if (daemon_)
        AUG_PERRINFO(aug_openlog(logfile_), NULL, "aug_openlog() failed");

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
    if (conffile) {
        aug_info("reading: %s", conffile);
        if (-1 == aug_readconf(conffile, setconfopt_, NULL))
            return -1;
        aug_strlcpy(conffile_, conffile, sizeof(conffile_));
    }

    daemon_ = daemon;

    /* Use working directory as default. */

    if (!*rundir_) {
        char cwd[AUG_PATH_MAX + 1];
        if (!aug_getcwd(cwd, sizeof(cwd)))
            return -1;
        if (!aug_realpath(rundir_, cwd, sizeof(rundir_)))
            return -1;
    }

    return reconf_();
}

static int
readevent_(int fd, const struct aug_var* arg, struct aug_files* files)
{
    struct aug_event event;

    AUG_DEBUG0("checking event pipe '%d'", fd);

    if (!aug_ioevents(mplexer_, fd))
        return 1;

    AUG_DEBUG0("reading event");
    if (!aug_readevent(aug_eventin(), &event))
        aug_perrinfo(NULL, "aug_readevent() failed");

    switch (event.type_) {
    case AUG_EVENTRECONF:
        aug_info("received AUG_EVENTRECONF");
        if (*conffile_) {
            aug_info("reading: %s", conffile_);
            if (-1 == aug_readconf(conffile_, setconfopt_, NULL))
                return -1;
        }
        AUG_PERRINFO(reconf_(), NULL, "failed to re-configure daemon");
        break;
    case AUG_EVENTSTATUS:
        aug_info("received AUG_EVENTSTATUS");
        break;
    case AUG_EVENTSTOP:
        aug_info("received AUG_EVENTSTOP");
        quit_ = 1;
        break;
    }
    aug_destroyvar(&event.arg_);
    return 1;
}

static int
init_(const struct aug_var* arg)
{
    struct aug_hostserv hs;
    struct aug_endpoint ep;
    struct aug_var ptr;

    aug_info("initialising daemon process");

    if (-1 == aug_setsrvlogger("aug") || !(mplexer_ = aug_createmplexer()))
        return -1;

    if (!aug_parsehostserv(address_, &hs))
        goto fail1;

    if (-1 == (fd_ = aug_tcplisten(hs.host_, hs.serv_, &ep)))
        goto fail1;

    if (-1 == aug_insertfile(&files_, fd_, listener_,
                             aug_setvarp(&ptr, &mplexer_, NULL)))
        goto fail2;

    if (-1 == aug_insertfile(&files_, aug_eventin(), readevent_, &ptr)
        || -1 == aug_setioeventmask(mplexer_, fd_, AUG_IOEVENTRD)
        || -1 == aug_setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD))
        goto fail3;

    return 0;

 fail3:
    aug_destroyfiles(&files_);
 fail2:
    aug_close(fd_);
 fail1:
    aug_destroymplexer(mplexer_);
    return -1;
}

static int
run_(const struct aug_var* arg)
{
    struct timeval timeout;

    aug_info("running daemon process");

    while (!AUG_EMPTY(&files_) && !quit_) {

        if (!AUG_EMPTY(&timers_))
            AUG_PERRINFO(aug_foreachexpired(&timers_, 0, &timeout), NULL,
                         "aug_foreachexpired() failed");

        /* If SA_RESTART has been set for an interrupting signal, it is
           implementation dependant whether select/poll restart or return with
           EINTR set. */

        while (-1 == aug_waitioevents(mplexer_, !AUG_EMPTY(&timers_)
                                      ? &timeout : NULL))
            aug_perrinfo(NULL, "aug_waitevents() failed");

        AUG_PERRINFO(aug_foreachfile(&files_), NULL,
                     "aug_foreachfile() failed");
    }
    return 0;
}

static void
term_(const struct aug_var* arg)
{
    aug_info("terminating daemon process");

    AUG_PERRINFO(aug_destroytimers(&timers_), NULL,
                 "aug_destroytimers() failed");
    AUG_PERRINFO(aug_destroyfiles(&files_), NULL,
                 "aug_destroyfiles() failed");
    AUG_PERRINFO(aug_close(fd_), NULL, "aug_close() failed");
    AUG_PERRINFO(aug_destroymplexer(mplexer_), NULL,
                 "aug_destroymplexer() failed");
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
        term_
    };
    struct aug_var arg = AUG_VARNULL;

    program_ = argv[0];
    service_ = &service;
    aug_atexitinit(&errinfo);
    return aug_main(argc, argv, &service, &arg);
}
