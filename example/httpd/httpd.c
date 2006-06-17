/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
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

#if defined(_MSC_VER)
# pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

#define PORT_ 8080

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
request_(void* arg, const char* initial, aug_mar_t mar,
         struct aug_messages* messages)
{
    char buf[64];
    time_t t;
    struct aug_field f;
    aug_dstr_t s = aug_createdstr(0);
    struct aug_message* message;
    aug_dstrsets(&s, "HTTP/1.0 200 OK");

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

    if (-1 == aug_seteventmask(mplexer, fd, AUG_EVENTRD)) {
        aug_freestate(state);
        return NULL;
    }

    return state;
}

static int
freeconn_(aug_state_t state, aug_mplexer_t mplexer, int fd)
{
    aug_seteventmask(mplexer, fd, 0);
    aug_freestate(state);
    return 0;
}

static int
conn_(void* arg, int fd, struct aug_conns* conns)
{
    ssize_t ret;
    aug_state_t state = (aug_state_t)arg;
    int events = aug_events(mplexer_, fd);

    AUG_DEBUG("checking connection '%d'", fd);

    if (events & AUG_EVENTRD) {

        AUG_DEBUG("handling read event");

        if (-1 == (ret = aug_readsome(state, fd))) {
            aug_perror("aug_readsome() failed");
            goto fail;
        }

        if (0 == ret) {
            aug_info("closing connection '%d'", fd);
            goto fail;
        }

        if (aug_pending(state))
            aug_seteventmask(mplexer_, fd, AUG_EVENTRD | AUG_EVENTWR);
    }

    if (events & AUG_EVENTWR) {

        AUG_DEBUG("handling write event");

        if (-1 == (ret = aug_writesome(state, fd))) {
            aug_perror("aug_writesome() failed");
            goto fail;
        }

        if (!aug_pending(state)) {
            aug_seteventmask(mplexer_, fd, AUG_EVENTRD);

            // For HTTP test.
            goto fail;
        }
    }

    return 1;

 fail:
    freeconn_(arg, mplexer_, fd);
    aug_close(fd);
    return 0;
}

static int
listener_(void* arg, int fd, struct aug_conns* conns)
{
    struct sockaddr_in addr;
    socklen_t len;
    int conn;

    AUG_DEBUG("checking listener '%d'", fd);

    if (!aug_events(mplexer_, fd))
        return 1;

    AUG_DEBUG("accepting new connection");

    len = sizeof(addr);
    if (-1 == (conn = aug_accept(fd, (struct sockaddr*)&addr, &len))) {
        aug_perror("aug_accept() failed");
        return 1;
    }

    aug_info("initialising new connection '%d'", conn);

    if (-1 == aug_setnonblock(conn, 1)) {
        aug_perror("aug_setnonblock() failed");
        aug_close(conn);
        return 1;
    }

    if (!(arg = createconn_(mplexer_, conn))) {
        aug_perror("failed to create connection");
        aug_close(conn);
        return 1;
    }

    aug_insertconn(conns, conn, conn_, arg);
    return 1;
}

static int
setconfopt_(void* arg, const char* name, const char* value)
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

static int
setopt_(void* arg, enum aug_option opt, const char* value)
{
    if (AUG_OPTCONFFILE != opt) {
        errno = EINVAL;
        return -1;
    }

    aug_strlcpy(conffile_, value, sizeof(conffile_));
    return 0;
}

static const char*
getopt_(void* arg, enum aug_option opt)
{
    switch (opt) {
    case AUG_OPTCONFFILE:
        return *conffile_ ? conffile_ : NULL;
    case AUG_OPTPIDFILE:
        return pidfile_;
    }
    return NULL;
}

static int
config_(void* arg, int daemon)
{
    daemon_ = daemon;

    if (*conffile_) {

        aug_info("reading: %s", conffile_);

        if (-1 == aug_readconf(conffile_, setconfopt_, NULL))
            return -1;
    }

    AUG_VERIFY(chdir(rundir_), "chdir() failed");

    if (daemon)
        AUG_VERIFY(aug_openlog(logfile_), "aug_openlog() failed");

    aug_info("run directory: %s", rundir_);
    aug_info("pid file: %s", pidfile_);
    aug_info("log file: %s", logfile_);
    aug_info("log level: %d", aug_loglevel());
    aug_info("address: %s", address_);
    return 0;
}

static int
pipe_(void* arg, int fd, struct aug_conns* conns)
{
    aug_sig_t sig;

    AUG_DEBUG("checking signal pipe '%d'", fd);

    if (!aug_events(mplexer_, fd))
        return 1;

    AUG_DEBUG("reading signal action");
    AUG_VERIFY(aug_readsig(&sig), "aug_readsig() failed");

    switch (sig) {
    case AUG_SIGRECONF:
        aug_info("received AUG_SIGRECONF");
        AUG_VERIFY(config_(arg, daemon_), "failed to re-configure daemon");
        break;
    case AUG_SIGSTATUS:
        aug_info("received AUG_SIGSTATUS");
        break;
    case AUG_SIGSTOP:
        aug_info("received AUG_SIGSTOP");
        quit_ = 1;
        break;
    }
    return 1;
}

static int
init_(void* arg)
{
    struct sockaddr_in addr;

    aug_info("initialising daemon process");

    if (-1 == aug_setsrvlogger("httpd")
        || !(mplexer_ = aug_createmplexer()))
        return -1;

    if (!aug_parseinet(&addr, address_))
        goto fail1;

    if (-1 == (fd_ = aug_openpassive(&addr)))
        goto fail1;

    if (-1 == aug_insertconn(&conns_, fd_, listener_, mplexer_))
        goto fail2;

    if (-1 == aug_insertconn(&conns_, aug_sigin(), pipe_, mplexer_)
        || -1 == aug_seteventmask(mplexer_, fd_, AUG_EVENTRD)
        || -1 == aug_seteventmask(mplexer_, aug_sigin(), AUG_EVENTRD))
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
run_(void* arg)
{
    struct timeval timeout;

    aug_info("running daemon process");

    while (!AUG_EMPTY(&conns_) && !quit_) {

        if (!AUG_EMPTY(&timers_))
            AUG_VERIFY(aug_processtimers(&timers_, 0, &timeout),
                       "aug_processtimers() failed");

        /* If SA_RESTART has been set for an interrupting signal, it is
           implementation dependant whether select/poll restart or return with
           EINTR set. */

        while (-1 == aug_waitevents(mplexer_, !AUG_EMPTY(&timers_)
                                    ? &timeout : NULL))
            aug_perror("aug_waitevents() failed");

        AUG_VERIFY(aug_processconns(&conns_),
                   "aug_processconns() failed");
    }

    aug_info("stopping daemon process");

    AUG_VERIFY(aug_freetimers(&timers_), "aug_freetimers() failed");
    AUG_VERIFY(aug_freeconns(&conns_), "aug_freeconns() failed");
    AUG_VERIFY(aug_close(fd_), "aug_close() failed");
    AUG_VERIFY(aug_freemplexer(mplexer_), "aug_freemplexer() failed");
    return 0;
}

static void
term_(void)
{
    aug_term();
}

int
main(int argc, char* argv[])
{
    struct aug_service service = {
        setopt_,
        getopt_,
        config_,
        init_,
        run_,
        argv[0],
        "HTTP Daemon",
        "httpd",
        "Mark Aylett <mark@emantic.co.uk>"
    };

    aug_init();
    atexit(term_);

    service_ = &service;
    if (!getcwd(rundir_, sizeof(rundir_))) {
        aug_perror("getcwd() failed");
        return 1;
    }

    aug_main(&service, argc, argv);
    return 1;
}
