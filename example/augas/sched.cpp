#include "augaspp.hpp"

#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include <map>
#include <sstream>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    enum tmtype { TMLOCAL, TMUTC };

    struct tmevent {
        const string name_, spec_;
        const tmtype type_;
        aug_tmspec tmspec_;
        tmevent(const string& name, const string& spec, tmtype type)
            : name_(name),
              spec_(spec),
              type_(type)
        {
        }
    };

    typedef smartptr<tmevent> tmeventptr;
    typedef map<time_t, tmeventptr> tmqueue;

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

            if (!aug_nexttime(aug_gmtime(&now, &tm), &ptr->tmspec_))
                return;

            q.insert(make_pair(now = aug_timegm(&tm), ptr));

        } else {

            if (!aug_nexttime(aug_localtime(&now, &tm), &ptr->tmspec_))
                return;

            q.insert(make_pair(now = aug_timelocal(&tm), ptr));
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
        string spec;
        while (is >> spec) {

            tmeventptr ptr(new tmevent(name, spec, type));
            if (aug_strtmspec(&ptr->tmspec_, spec.c_str()))
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

        tmqueue::const_iterator next(q.begin());
        timeval expiry = { next->first, 0 };
        tvsub(expiry, tv);
        unsigned ms(tvtoms(expiry));
        return ms < 60000 ? ms : 60000;
    }

    struct schedserv : basic_serv {
        augas_id timer_;
        tmqueue queue_;
        bool
        do_start(const char* sname)
        {
            do_reconf();
            return true;
        }
        void
        do_reconf()
        {
            queue_.clear();

            timeval tv;
            gettimeofday(tv);
            time_t now(tv.tv_sec);
            pushevents(queue_, now);
            unsigned ms(timerms(queue_, tv));
            if (ms) {
                if (-1 != timer_)
                    resettimer(timer_, ms);
                else {
                    augas_var var = AUG_VARNULL;
                    timer_ = settimer(ms, var);
                }
                aug_info("next expiry in %d ms", ms);
            } else if (-1 != timer_) {
                canceltimer(timer_);
                timer_ = -1;
            }
        }
        void
        do_event(const char* from, const char* type, const void* user,
                 size_t size)
        {
            aug_info("event [%s] triggered", type);

            stringstream ss;
            ss << "<events>";
            tmqueue::const_iterator it(queue_.begin()), end(queue_.end());
            for (; it != end; ++it) {
                tm tm;
                ss << "<event name=\"" << it->second->name_
                   << "\" spec=\"" << it->second->spec_ << "\" tz=\""
                   << (TMUTC == it->second->type_ ? "utc" : "local")
                   << "\">" << tmstring(*aug_localtime(&it->first,
                                                                 &tm))
                   << "</event>";
            }
            ss << "</events>";

            dispatch("http", "response", ss.str().c_str(), ss.str().size());
        }
        void
        do_expire(const object& timer, unsigned& ms)
        {
            timeval tv;
            gettimeofday(tv);
            time_t now(tv.tv_sec);

            while (!queue_.empty() && queue_.begin()->first <= now) {

                tmeventptr ptr(queue_.begin()->second);
                queue_.erase(queue_.begin());
                augas_post("schedclient", ptr->name_.c_str(), 0);
                pushevent(queue_, now, ptr);
            }

            ms = timerms(queue_, tv);
            aug_info("next expiry in %d ms", ms);
        }
        ~schedserv() AUGAS_NOTHROW
        {
        }
        schedserv()
            : timer_(-1)
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
