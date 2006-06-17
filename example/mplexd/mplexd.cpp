/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "augsrv.h"
#include "augsys.h"

#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>

#include <time.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#if defined(_MSC_VER)
# pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

using namespace aug;
using namespace boost;
using namespace std;

namespace test {

    char conffile_[AUG_PATH_MAX + 1] = "";
    char rundir_[AUG_PATH_MAX + 1];
    char pidfile_[AUG_PATH_MAX + 1] = "mplexd.pid";
    char logfile_[AUG_PATH_MAX + 1] = "mplexd.log";
    char address_[AUG_PATH_MAX + 1] = "127.0.0.1:8080";

    void
    setnodelay(fdref ref, bool on)
    {
        int value(on ? 1 : 0);
        setsockopt(ref, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
    }

    class setopt : public setopt_base {
        void
        do_setopt(const char* name, const char* value)
        {
            if (0 == aug_strcasecmp(name, "address")) {

                aug_strlcpy(address_, value, sizeof(address_));

            } else if (0 == aug_strcasecmp(name, "loglevel")) {

                unsigned int level(strtoui(value, 10));
                aug_info("setting log level: %d", level);
                aug_setloglevel(level);

            } else if (0 == aug_strcasecmp(name, "logfile")) {

                aug_strlcpy(logfile_, value, sizeof(logfile_));

            } else if (0 == aug_strcasecmp(name, "pidfile")) {

                aug_strlcpy(pidfile_, value, sizeof(pidfile_));

            } else if (0 == aug_strcasecmp(name, "rundir")) {

                realpath(rundir_, value, sizeof(rundir_));

            } else {

                string s("option not supported: ");
                s += name;
                error(s, EINVAL);
            }
        }
    };

    bool daemon_(false);
    bool quit_(false);

    void
    config(bool daemon)
    {
        daemon_ = daemon;

        if (*conffile_) {

            aug_info("reading: %s", conffile_);

            test::setopt action;
            readconf(conffile_, action);
        }

        if (-1 == chdir(rundir_))
            error("chdir() failed");

        if (daemon)
            openlog(logfile_);

        aug_info("run directory: %s", rundir_);
        aug_info("pid file: %s", pidfile_);
        aug_info("log file: %s", logfile_);
        aug_info("log level: %d", aug_loglevel());
    }

    void
    hook(int fd, int type, void* data)
    {
        try {
            mplexer* mp(static_cast<mplexer*>(data));
            seteventmask(*mp, fd, 0);
        }
        AUG_CATCHRETURN;
    }

    class buffer {
        vector<char> vec_;
        size_t begin_, end_;
    public:
        buffer()
            : vec_(1024),
              begin_(0),
              end_(0)
        {
        }
        void
        putsome(const void* buf, size_t size)
        {
            if (vec_.size() - end_ < size)
                vec_.resize(end_ + size);

            memcpy(&vec_[end_], buf, size);
            end_ += size;
        }
        bool
        readsome(fdref ref)
        {
            char buf[4096];
            size_t size(read(ref, buf, sizeof(buf) - 1));
            if (0 == size)
                return false;

            putsome(buf, size);
            return true;
        }
        bool
        writesome(fdref ref)
        {
            size_t size(end_ - begin_);
            size = write(ref, &vec_[begin_], size);
            if ((begin_ += size) == end_) {
                begin_ = end_ = 0;
                return false;
            }
            return true;
        }
        bool
        empty() const
        {
            return begin_ == end_;
        }
    };

    struct cstate : public expire_base {

        smartfd sfd_;
        timer timer_;
        buffer buffer_;
        int heartbeats_;

        void
        do_expire(int id)
        {
            aug_info("timeout");
            if (heartbeats_ < 3) {
                buffer_.putsome("heartbeat\n", 10);
                ++heartbeats_;
                mplexer* mp(static_cast<mplexer*>(fddata(sfd_)));
                seteventmask(*mp, sfd_, AUG_EVENTRDWR);
            } else
                shutdown(sfd_, SHUT_RDWR);
        }
        ~cstate() NOTHROW
        {
        }
        explicit
        cstate(const smartfd& sfd, timers& timers)
            : sfd_(sfd),
              timer_(timers, 5000, *this),
              heartbeats_(0)
        {
        }
    };

    typedef shared_ptr<cstate> cstateptr;

    struct sstate {

        conns conns_;
        timers timers_;
        mplexer mplexer_;
        smartfd sfd_;
        map<int, cstateptr> sfds_;

        explicit
        sstate(poll_base& poll)
            : sfd_(null)
        {
            struct sockaddr_in addr;
            smartfd sfd(openpassive(parseinet(addr, address_)));

            insertconn(conns_, aug_sigin(), poll);
            seteventmask(mplexer_, aug_sigin(), AUG_EVENTRD);

            insertconn(conns_, sfd, poll);
            seteventmask(mplexer_, sfd, AUG_EVENTRD);

            sfd_ = sfd;
        }
    };

    class service : public poll_base, public service_base {

        auto_ptr<sstate> state_;

        void
        setfdhook(fdref ref, unsigned short mask)
        {
            insertconn(state_->conns_, ref, *this);
            try {
                aug::setfdhook(ref, hook, &state_->mplexer_);
                seteventmask(state_->mplexer_, ref, mask);
            } catch (...) {
                removeconn(state_->conns_, ref);
            }
        }

        bool
        signaller(int fd, struct aug_conns& conns)
        {
            AUG_DEBUG("reading signal action");

            switch (readsig()) {
            case AUG_SIGRECONF:
                aug_info("received AUG_SIGRECONF");
                config(daemon_);
                break;
            case AUG_SIGSTATUS:
                aug_info("received AUG_SIGSTATUS");
                break;
            case AUG_SIGSTOP:
                aug_info("received AUG_SIGSTOP");
                quit_ = true;
                break;
            }
            return true;
        }

        bool
        listener(int fd, struct aug_conns& conns)
        {
            struct sockaddr_in addr;
            socklen_t len(sizeof(addr));

            AUG_DEBUG("accepting connection");

            smartfd sfd(accept(fd, *reinterpret_cast<sockaddr*>(&addr), len));

            aug_info("initialising connection '%d'", sfd.get());

            setnonblock(sfd, true);
            setfdhook(sfd, AUG_EVENTRD);
            state_->sfds_.insert(make_pair
                                 (sfd.get(), cstateptr
                                  (new cstate(sfd, state_->timers_))));
            return true;
        }

        bool
        connection(int fd, struct aug_conns& conns)
        {
            cstateptr ptr(state_->sfds_[fd]);
            unsigned short bits(events(state_->mplexer_, fd));

            if (bits & AUG_EVENTRD) {

                AUG_DEBUG("handling read event '%d'", fd);

                if (!ptr->buffer_.readsome(fd)) {

                    aug_info("closing connection '%d'", fd);
                    state_->sfds_.erase(fd);
                    return false;
                }

                seteventmask(state_->mplexer_, fd, AUG_EVENTRDWR);
                ptr->timer_.cancel();
                ptr->heartbeats_ = 0;
            }

            if (bits & AUG_EVENTWR) {

                if (!ptr->buffer_.writesome(fd)) {
                    seteventmask(state_->mplexer_, fd, AUG_EVENTRD);
                    ptr->timer_.reset(5000);
                }
            }

            return true;
        }

        bool
        do_poll(int fd, struct aug_conns& conns)
        {
            if (!events(state_->mplexer_, fd))
                return true;

            if (fd == aug_sigin())
                return signaller(fd, conns);

            if (fd == state_->sfd_)
                return listener(fd, conns);

            return connection(fd, conns);
        }

        void
        do_setopt(enum aug_option opt, const char* value)
        {
            if (AUG_OPTCONFFILE != opt)
                error("unsupported option", EINVAL);

            aug_strlcpy(conffile_, value, sizeof(conffile_));
        }

        const char*
        do_getopt(enum aug_option opt)
        {
            switch (opt) {
            case AUG_OPTCONFFILE:
                return *conffile_ ? conffile_ : 0;
            case AUG_OPTPIDFILE:
                return pidfile_;
            }
            return 0;
        }

        void
        do_config(bool daemon)
        {
            test::config(daemon);
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            setsrvlogger("mplexd");
            state_.reset(new sstate(*this));
        }

        void
        do_run()
        {
            struct timeval tv;

            aug_info("running daemon process");

            int ret(!0);
            while (!quit_) {

                if (state_->timers_.empty()) {

                    sigunblock();
                    while (AUG_EINTR == (ret = waitevents(state_->mplexer_)))
                        ;
                    sigblock();

                } else {

                    process(state_->timers_, 0 == ret, tv);

                    sigunblock();
                    while (AUG_EINTR == (ret = waitevents(state_
                                                          ->mplexer_, tv)))
                        ;
                    sigblock();
                }

                processconns(state_->conns_);
            }

            aug_info("stopping daemon process");
        }

    public:
        ~service() NOTHROW
        {
        }
        service()
        {
        }
    };

    string
    getcwd()
    {
        char buf[AUG_PATH_MAX + 1];
        if (!::getcwd(buf, sizeof(buf)))
            error("getcwd() failed");

        return buf;
    }
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        initialiser init;

        aug_setloglevel(AUG_LOGINFO);
        sigblock();
        service s;

        if (!getcwd(rundir_, sizeof(rundir_)))
            error("getcwd() failed");

        main(s, argv[0], "Multiplexer Daemon", "mplexd",
             "Mark Aylett <mark@emantic.co.uk>", argc, argv);

    } catch (const exception& e) {

        aug_error("%s", e.what());
    }

    return 1; // aug_main() does not return.
}

#if 0
#include "message.hpp"
#include "state.hpp"

#include "augsrv/control.h"
#include "augsrv/daemon.h"
#include "augsrv/global.h"
#include "augsrv/log.h"
#include "augsrv/main.h"
#include "augsrv/options.h"
#include "augsrv/signal.h"
#include "augsrv/types.h"

#include "augnet/conn.h"
#include "augnet/inet.h"
#include "augnet/parser.h"

#include "augutil/conv.h"
#include "augutil/file.h"
#include "augutil/path.h"
#include "augutil/timer.h"

#include "augmar/mar.h"

#include "augsys/base.h"
#include "augsys/defs.h"
#include "augsys/errno.h"
#include "augsys/inet.h"
#include "augsys/limits.h"
#include "augsys/log.h"
#include "augsys/mplexer.h"
#include "augsys/socket.h"
#include "augsys/string.h"
#include "augsys/unistd.h"
#include "augsys/utility.h"

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
static char pidfile_[AUG_PATH_MAX + 1] = "mplexd.pid";
static char logfile_[AUG_PATH_MAX + 1] = "mplexd.log";
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
    freeconn_(state, mplexer_, fd);
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
    enum aug_signal sig = AUG_SIGSTOP;

    AUG_DEBUG("checking signal pipe '%d'", fd);

    if (!aug_events(mplexer_, fd))
        return 1;

    AUG_DEBUG("reading signal action");
    AUG_VERIFY(aug_readsig(&sig), "aug_readsig() failed");

    switch (sig) {
    case AUG_SIGSYSTEM:
        aug_info("received AUG_SIGSYSTEM");
        break;
    case AUG_SIGALARM:
        aug_info("received AUG_SIGALARM");
        break;
    case AUG_SIGCHILD:
        aug_info("received AUG_SIGCHILD");
        break;
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
    case AUG_SIGUSER:
        aug_info("received AUG_SIGUSER");
        break;
    }
    return 1;
}

static int
init_(void* arg)
{
    struct sockaddr_in addr;

    aug_info("initialising daemon process");

    if (-1 == aug_setsrvlogger("mplexd")
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
        "Multiplexer Daemon",
        "mplexd",
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
#endif
