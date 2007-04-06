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

    typedef pair<time_t, smartptr<aug_tmspec> > tmpair;

    struct tmless {
        bool
        operator ()(const tmpair& lhs, const tmpair& rhs)
        {
            return lhs.first < rhs.first;
        }
    };

    typedef priority_queue<tmpair, deque<tmpair>, tmless> tmqueue;

    void
    pushevent(tmqueue& q, const string& name, tm& local)
    {
        const char* tmspecs(augas::getenv(string("service.sched.event.")
                                          .append(name).c_str()));
        if (!tmspecs)
            return;

        istringstream is(tmspecs);
        string tmspec;
        while (is >> tmspec) {

            smartptr<aug_tmspec> sptr(new aug_tmspec());
            if (aug_strtmspec(sptr.get(), tmspec.c_str())
                && aug_nexttime(&local, sptr.get())) {

                aug_info("event scheduled");
                q.push(make_pair(mktime(&local), sptr));
            }
        }
    }

    void
    pushevents(tmqueue& q, time_t now)
    {
        tm* local(localtime(&now));

        const char* events(augas::getenv("service.sched.events"));
        if (!events)
            return;

        istringstream is(events);
        string name;
        while (is >> name)
            pushevent(q, name, *local);
    }

    unsigned
    timerms(tmqueue& q, const timeval& tv)
    {
        if (q.empty())
            return 0;

        const tmpair& next(q.top());
        aug_info("%s", ctime(&tv.tv_sec));
        aug_info("%s", ctime(&next.first));
        timeval expiry = { next.first, 0 };
        tvsub(expiry, tv);
        unsigned ms(tvtoms(expiry));
        return ms < 5000 ? ms : 5000;
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
            }
            return true;
        }
        void
        do_expire(const object& timer, unsigned& ms)
        {
            timeval tv;
            gettimeofday(tv);
            time_t now(tv.tv_sec);
            while (queue_.top().first <= now) {
                aug_info("event triggered");
                queue_.pop();
            }
            ms = timerms(queue_, tv);
            aug_info("next expiry in %d ms", ms);
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
