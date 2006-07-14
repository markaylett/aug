/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

static const char rcsid[] = "$Id:$";

#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>

#include <time.h>

using namespace aug;
using namespace boost;
using namespace std;

namespace test {

    typedef char cstring[AUG_PATH_MAX + 1];

    const char* program_;

    cstring conffile_= "";
    cstring rundir_;
    cstring pidfile_ = "mplexd.pid";
    cstring logfile_ = "mplexd.log";
    cstring address_ = "127.0.0.1:8080";

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
                throw runtime_error("option not supported");
            }
        }
    };

    bool daemon_(false);
    bool quit_(false);

    void
    reconfig()
    {
        if (*conffile_) {

            aug_info("reading: %s", conffile_);

            test::setopt action;
            readconf(conffile_, action);
        }

        if (-1 == chdir(rundir_)) {
            aug_setposixerrinfo(__FILE__, __LINE__, errno);
            throwerrinfo("chdir() failed");
        }

        if (daemon_)
            openlog(logfile_);

        aug_info("run directory: %s", rundir_);
        aug_info("pid file: %s", pidfile_);
        aug_info("log file: %s", logfile_);
        aug_info("log level: %d", aug_loglevel());
    }

    void
    config(const char* conffile, bool daemon)
    {
        if (conffile)
            realpath(conffile_, conffile, sizeof(conffile_));

        daemon_ = daemon;
        reconfig();
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

    struct session : public expire_base {

        mplexer& mplexer_;
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
                setioeventmask(mplexer_, sfd_, AUG_IOEVENTRDWR);
            } else
                shutdown(sfd_, SHUT_RDWR);
        }
        ~session() NOTHROW
        {
            try {
                setioeventmask(mplexer_, sfd_, 0);
            } AUG_PERRINFOCATCH;
        }
        session(mplexer& mplexer, const smartfd& sfd, timers& timers)
            : mplexer_(mplexer),
              sfd_(sfd),
              timer_(timers, 5000, *this),
              heartbeats_(0)
        {
        }
    };

    typedef shared_ptr<session> sessionptr;

    struct state {

        conns conns_;
        timers timers_;
        mplexer mplexer_;
        smartfd sfd_;
        map<int, sessionptr> sfds_;

        explicit
        state(poll_base& poll)
            : sfd_(null)
        {
            struct sockaddr_in addr;
            smartfd sfd(tcplisten(parseinet(addr, address_)));

            insertconn(conns_, aug_eventin(), poll);
            setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD);

            insertconn(conns_, sfd, poll);
            setioeventmask(mplexer_, sfd, AUG_IOEVENTRD);

            sfd_ = sfd;
        }
    };

    class service : public poll_base, public service_base {

        auto_ptr<state> state_;

        void
        setfdhook(fdref ref, unsigned short mask)
        {
            insertconn(state_->conns_, ref, *this);
            try {
                setioeventmask(state_->mplexer_, ref, mask);
            } catch (...) {
                removeconn(state_->conns_, ref);
            }
        }

        bool
        readevent(int fd, struct aug_conns& conns)
        {
            struct aug_event event;
            AUG_DEBUG("reading event");

            switch (aug::readevent(aug_eventin(), event).type_) {
            case AUG_EVENTRECONF:
                aug_info("received AUG_EVENTRECONF");
                reconfig();
                break;
            case AUG_EVENTSTATUS:
                aug_info("received AUG_EVENTSTATUS");
                break;
            case AUG_EVENTSTOP:
                aug_info("received AUG_EVENTSTOP");
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

            smartfd sfd(null);
            try {

                // As prescribed by Stevens: the passive socket is
                // non-blocking and certain errors ignored.  This is to handle
                // the situation where a client closes before the server has
                // had a chance to accept.

                sfd = accept(state_->sfd_,
                             *reinterpret_cast<sockaddr*>(&addr), len);

            } catch (const errinfo_error& e) {

                if (AUG_SRCPOSIX == aug_errsrc)
                    switch (aug_errnum) {
                    case ECONNABORTED:
#if !defined(_WIN32)
                    case EPROTO:
#endif // !_WIN32
                    case EWOULDBLOCK:
                        aug_warn("accept() failed: %s", e.what());
                        return true;
                    }

                throw;
            }

            aug_info("initialising connection '%d'", sfd.get());

            setnodelay(sfd, true);
            setnonblock(sfd, true);
            setfdhook(sfd, AUG_IOEVENTRD);

            state_->sfds_.insert(make_pair
                                 (sfd.get(), sessionptr
                                  (new session(state_->mplexer_, sfd,
                                               state_->timers_))));
            return true;
        }

        bool
        connection(int fd, struct aug_conns& conns)
        {
            sessionptr ptr(state_->sfds_[fd]);
            unsigned short bits(ioevents(state_->mplexer_, fd));

            if (bits & AUG_IOEVENTRD) {

                AUG_DEBUG("handling read event '%d'", fd);

                if (!ptr->buffer_.readsome(fd)) {

                    aug_info("closing connection '%d'", fd);
                    state_->sfds_.erase(fd);
                    return false;
                }

                setioeventmask(state_->mplexer_, fd, AUG_IOEVENTRDWR);
                ptr->timer_.cancel();
                ptr->heartbeats_ = 0;
            }

            if (bits & AUG_IOEVENTWR) {

                if (!ptr->buffer_.writesome(fd)) {
                    setioeventmask(state_->mplexer_, fd, AUG_IOEVENTRD);
                    ptr->timer_.reset(5000);
                }
            }

            return true;
        }

        bool
        do_poll(int fd, struct aug_conns& conns)
        {
            if (!ioevents(state_->mplexer_, fd))
                return true;

            if (fd == aug_eventin())
                return readevent(fd, conns);

            if (fd == state_->sfd_)
                return listener(fd, conns);

            return connection(fd, conns);
        }

        const char*
        do_getopt(enum aug_option opt)
        {
            switch (opt) {
            case AUG_OPTADMIN:
                return "Mark Aylett <mark@emantic.co.uk>";
            case AUG_OPTCONFFILE:
                return *conffile_ ? conffile_ : 0;
            case AUG_OPTLONGNAME:
                return "Multiplexer Daemon";
            case AUG_OPTPIDFILE:
                return pidfile_;
            case AUG_OPTPROGRAM:
                return program_;
            case AUG_OPTSHORTNAME:
                return "mplexd";
            }
            return 0;
        }

        void
        do_config(const char* conffile, bool daemon)
        {
            test::config(conffile, daemon);
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            setsrvlogger("aug");
            state_.reset(new state(*this));
        }

        void
        do_run()
        {
            struct timeval tv;

            aug_info("running daemon process");

            int ret(!0);
            while (!quit_) {

                if (state_->timers_.empty()) {

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_)))
                        ;

                } else {

                    processtimers(state_->timers_, 0 == ret, tv);

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_,
                                                              tv)))
                        ;
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
    } service_;

    string
    getcwd()
    {
        cstring buf;
        if (!::getcwd(buf, sizeof(buf))) {
            aug_setposixerrinfo(__FILE__, __LINE__, errno);
            throwerrinfo("getcwd() failed");
        }

        return buf;
    }
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        struct aug_errinfo errinfo;
        scoped_init init(errinfo);

        program_ = argv[0];

        blocksignals();
        aug_setloglevel(AUG_LOGINFO);

        if (!getcwd(rundir_, sizeof(rundir_))) {
            aug_setposixerrinfo(__FILE__, __LINE__, errno);
            throwerrinfo("getcwd() failed");
        }

        main(service_, argc, argv);

    } catch (const exception& e) {

        aug_error("%s", e.what());
    }

    return 1; // aug_main() does not return.
}
