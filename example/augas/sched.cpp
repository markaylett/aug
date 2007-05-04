#include "augaspp.hpp"

#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include <map>
#include <sstream>

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    enum tmtz { TMLOCAL, TMUTC };

    struct tmevent {
        const augas_id id_;
        const string name_, spec_;
        const tmtz tz_;
        aug_tmspec tmspec_;
        tmevent(augas_id id, const string& name, const string& spec, tmtz tz)
            : id_(id),
              name_(name),
              spec_(spec),
              tz_(tz)
        {
        }
        tmevent(const string& name, const string& spec, tmtz tz)
            : id_(aug_nextid()),
              name_(name),
              spec_(spec),
              tz_(tz)
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
        if (TMUTC == ptr->tz_) {

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
    pushevent(tmqueue& q, time_t now, const string& name, tmtz tz)
    {
        const char* tmspecs
            (augas::getenv(string("service.sched.event.").append(name)
                           .append(TMUTC == tz ? ".utc" : ".local")
                           .c_str()));
        if (!tmspecs)
            return;

        istringstream is(tmspecs);
        string spec;
        while (is >> spec) {

            tmeventptr ptr(new tmevent(name, spec, tz));
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

    void
    eraseevent(tmqueue& q, augas_id id)
    {
        tmqueue::iterator it(q.begin()), end(q.end());
        for (; it != end; ++it)
            if (it->second->id_ == id) {
                q.erase(it);
                break;
            }
    }

    unsigned
    timerms(const tmqueue& q, const timeval& tv)
    {
        if (q.empty())
            return 0;

        tmqueue::const_iterator next(q.begin());
        timeval expiry = { next->first, 0 };
        tvsub(expiry, tv);
        unsigned ms(tvtoms(expiry));
        return ms < 60000 ? ms : 60000;
    }

    ostream&
    toxml(ostream& os, const tmqueue& q)
    {
        os << "<events>";
        tmqueue::const_iterator it(q.begin()), end(q.end());
        for (; it != end; ++it) {
            tm tm;
            os << "<event id=\"" << it->second->id_
               << "\" name=\"" << it->second->name_
               << "\" spec=\"" << it->second->spec_ << "\" tz=\""
               << (TMUTC == it->second->tz_ ? "utc" : "local")
               << "\">" << tmstring(*aug_localtime(&it->first, &tm))
               << "</event>";
        }
        os << "</events>";
    }

    struct schedserv : basic_serv {
        augas_id timer_;
        tmqueue queue_;
        void
        checkexpired(const timeval& tv)
        {
            time_t now(tv.tv_sec);

            while (!queue_.empty() && queue_.begin()->first <= now) {

                tmeventptr ptr(queue_.begin()->second);
                queue_.erase(queue_.begin());
                augas_post("schedclient", ptr->name_.c_str(), 0);
                pushevent(queue_, now, ptr);
            }
        }
        void
        settimer(const timeval& tv)
        {
            checkexpired(tv);

            unsigned ms(timerms(queue_, tv));
            if (ms) {
                if (-1 != timer_)
                    resettimer(timer_, ms);
                else {
                    augas_var var = AUG_VARNULL;
                    timer_ = augas::settimer(ms, var);
                }
                aug_info("next expiry in %d ms", ms);
            } else if (-1 != timer_) {
                canceltimer(timer_);
                timer_ = -1;
            }
        }
        void
        delevent(const string& urlencoded)
        {
            map<string, string> params;
            urlunpack(urlencoded.begin(), urlencoded.end(),
                      inserter(params, params.begin()));

            augas_id id(atoi(params["id"].c_str()));
            aug_info("deleting event: id=[%d]", (int)id);

            eraseevent(queue_, id);

            timeval tv;
            gettimeofday(tv);
            settimer(tv);
        }
        void
        putevent(const string& urlencoded)
        {
            map<string, string> params;
            urlunpack(urlencoded.begin(), urlencoded.end(),
                      inserter(params, params.begin()));

            augas_id id(atoi(params["id"].c_str()));
            if (id) {
                aug_info("updating event: id=[%d]", id);
                eraseevent(queue_, id);
            } else {
                aug_info("inserting new event");
                id = aug_nextid();
            }

            const string& spec(params["spec"]);

            tmtz tz(TMUTC);
            if (params["tz"] == "local")
                tz = TMLOCAL;

            timeval tv;
            gettimeofday(tv);

            tmeventptr ptr(new tmevent(id, params["name"], spec, tz));
            if (aug_strtmspec(&ptr->tmspec_, spec.c_str()))
                pushevent(queue_, tv.tv_sec, ptr);

            settimer(tv);
        }
        bool
        do_start(const char* sname)
        {
            do_reconf();
            return true;
        }
        void
        do_reconf()
        {
            timeval tv;
            gettimeofday(tv);

            queue_.clear();
            pushevents(queue_, tv.tv_sec);
            settimer(tv);
        }
        void
        do_event(const char* from, const char* type, const void* user,
                 size_t size)
        {
            aug_info("event [%s] triggered", type);

            map<string, string> fields;
            const char* encoded(static_cast<const char*>(user));
            urlunpack(encoded, encoded + size,
                      inserter(fields, fields.begin()));

            map<string, string>::iterator it(fields.find("content"));
            if (it != fields.end() && !it->second.empty())
                it->second = filterbase64(it->second.data(),
                                          it->second.size(), AUG_DECODE64);

            map<string, string>::const_iterator jt(fields.begin()),
                end(fields.end());
            for (; jt != end; ++jt)
                writelog(AUGAS_LOGINFO, "%s=%s", jt->first.c_str(),
                         jt->second.c_str());

            if (0 == strcmp(type, "http.sched.delevent")) {
                delevent(fields["content"]);
            } else if (0 == strcmp(type, "http.sched.putevent")) {
                putevent(fields["content"]);
            } else if (0 != strcmp(type, "http.sched.events"))
                return;

            stringstream ss;
            toxml(ss, queue_);
            dispatch("http", "response", ss.str().c_str(), ss.str().size());
        }
        void
        do_expire(const object& timer, unsigned& ms)
        {
            timeval tv;
            gettimeofday(tv);

            checkexpired(tv);

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
