#include "augaspp.hpp"

#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include <deque>
#include <queue>
#include <sstream>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    struct tmevent {
        const string name_;
        aug_tmspec spec_;
        explicit
        tmevent(const string& name)
            : name_(name)
        {
        }
    };

    typedef pair<time_t, smartptr<tmevent> > tmpair;

    struct tmgreater {
        bool
        operator ()(const tmpair& lhs, const tmpair& rhs)
        {
            return rhs.first < lhs.first;
        }
    };

    typedef priority_queue<tmpair, deque<tmpair>, tmgreater> tmqueue;

    string
    tmstring(const tm& local)
    {
        char buf[256];
        strftime(buf, sizeof(buf), "%b %d %H:%M:%S", &local);
        return buf;
    }

    void
    pushevent(tmqueue& q, const string& name, time_t now)
    {
        const char* tmspecs(augas::getenv(string("service.sched.event.")
                                          .append(name).c_str()));
        if (!tmspecs)
            return;

        istringstream is(tmspecs);
        string tmspec;
        while (is >> tmspec) {

            tm* local(localtime(&now));
            smartptr<tmevent> sptr(new tmevent(name));

            if (aug_strtmspec(&sptr->spec_, tmspec.c_str())
                && aug_nexttime(local, &sptr->spec_)) {

                q.push(make_pair(mktime(local), sptr));
                aug_info("event scheduled: %s at %s", name.c_str(),
                         tmstring(*local).c_str());
            }
        }
    }

    void
    pushevents(tmqueue& q, time_t now)
    {
        const char* events(augas::getenv("service.sched.events"));
        if (!events)
            return;

        istringstream is(events);
        string name;
        while (is >> name)
            pushevent(q, name, now);
    }

    unsigned
    timerms(tmqueue& q, const timeval& tv)
    {
        if (q.empty())
            return 0;

        const tmpair& next(q.top());
        timeval expiry = { next.first, 0 };
        tvsub(expiry, tv);
        unsigned ms(tvtoms(expiry));
        return ms < 60000 ? ms : 60000;
    }

    struct schedserv : basic_serv {
        tmqueue queue_;
        bool
        do_start(const char* sname)
        {
            timeval tv;
            gettimeofday(tv);
            time_t now(tv.tv_sec);
            pushevents(queue_, now);
            unsigned ms(timerms(queue_, tv));
            if (ms) {
                augas_var var = AUG_VARNULL;
                settimer(ms, var);
                aug_info("next expiry in %d ms", ms);
            }
            return true;
        }
        void
        do_expire(const object& timer, unsigned& ms)
        {
            timeval tv;
            gettimeofday(tv);
            time_t now(tv.tv_sec);

            while (!queue_.empty() && queue_.top().first <= now) {

                tm* local(localtime(&now));
                smartptr<tmevent> sptr(queue_.top().second);
                queue_.pop();

                aug_info("event triggered: %s at %s",
                         sptr->name_.c_str(), tmstring(*local).c_str());
                if (aug_nexttime(local, &sptr->spec_))
                    queue_.push(make_pair(mktime(local), sptr));
            }

            ms = timerms(queue_, tv);
            aug_info("next expiry in %d ms", ms);
        }
        ~schedserv() AUGAS_NOTHROW
        {
        }
        static serv_base*
        create(const char* sname)
        {
            return new schedserv();
        }
    };

    typedef basic_module<basic_factory<schedserv> > module;
}

AUGAS_MODULE(module::init, module::term)
