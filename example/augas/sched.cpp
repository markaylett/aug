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

    enum tmtype { TMLOCAL, TMUTC };

    struct tmevent {
        const string name_;
        const tmtype type_;
        aug_tmspec spec_;
        explicit
        tmevent(const string& name, tmtype type)
            : name_(name),
              type_(type)
        {
        }
    };

    typedef smartptr<tmevent> tmeventptr;
    typedef pair<time_t, tmeventptr> tmpair;

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
    pushevent(tmqueue& q, time_t now, const tmeventptr& ptr)
    {
        tm tm;
        if (TMUTC == ptr->type_) {

            if (!aug_nexttime(aug_gmtime(&now, &tm), &ptr->spec_))
                return;

            q.push(make_pair(now = aug_timegm(&tm), ptr));

        } else {

            if (!aug_nexttime(aug_localtime(&now, &tm), &ptr->spec_))
                return;

            q.push(make_pair(now = aug_timelocal(&tm), ptr));
        }

        aug_info("event [%s] scheduled for %s", ptr->name_.c_str(),
                 tmstring(*aug_localtime(&now, &tm)).c_str());
    }

    void
    pushevent(tmqueue& q, time_t now, const string& name, tmtype type)
    {
        const char* tmspecs
            (augas::getenv(string("service.sched.event.").append(name)
                           .append(TMUTC == type ? ".utc" : ".local")
                           .c_str()));
        if (!tmspecs)
            return;

        istringstream is(tmspecs);
        string tmspec;
        while (is >> tmspec) {

            tmeventptr ptr(new tmevent(name, type));
            if (aug_strtmspec(&ptr->spec_, tmspec.c_str()))
                pushevent(q, now, ptr);
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
        while (is >> name) {
            pushevent(q, now, name, TMLOCAL);
            pushevent(q, now, name, TMUTC);
        }
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
        do_event(const char* from, const char* type, const void* user,
                 size_t size)
        {
            aug_info("event [%s] triggered", type);
        }
        void
        do_expire(const object& timer, unsigned& ms)
        {
            timeval tv;
            gettimeofday(tv);
            time_t now(tv.tv_sec);

            while (!queue_.empty() && queue_.top().first <= now) {

                tmeventptr ptr(queue_.top().second);
                queue_.pop();
                augas_post("schedclient", ptr->name_.c_str(), 0);
                pushevent(queue_, now, ptr);
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
