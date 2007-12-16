/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MAUD_BUILD
#include "maudpp.hpp"

#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include <map>
#include <sstream>

using namespace aug;
using namespace maud;
using namespace std;

namespace {

    enum tmtz { TMLOCAL, TMUTC };

    struct tmevent {
        const maud_id id_;
        const string name_, spec_;
        const tmtz tz_;
        aug_tmspec tmspec_;
        tmevent(maud_id id, const string& name, const string& spec, tmtz tz)
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
    typedef multimap<time_t, tmeventptr> tmqueue;

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
            (maud::getenv(string("session.sched.event.").append(name)
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
        const char* events(maud::getenv("session.sched.events"));
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
    eraseevent(tmqueue& q, maud_id id)
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
    toxml(ostream& os, const tmqueue& q, unsigned offset, unsigned max_)
    {
        offset = AUG_MIN(offset, q.size());

        os << "<events offset=\"" << offset
           << "\" total=\"" << static_cast<unsigned>(q.size()) << "\">";
        tmqueue::const_iterator it(q.begin()), end(q.end());
        for (advance(it, offset); it != end && max_; ++it, --max_) {
            tm tm;
            os << "<event id=\"" << it->second->id_
               << "\" name=\"" << it->second->name_
               << "\" spec=\"" << it->second->spec_ << "\" tz=\""
               << (TMUTC == it->second->tz_ ? "utc" : "local")
               << "\">" << tmstring(*aug_localtime(&it->first, &tm))
               << "</event>";
        }
        os << "</events>";
        return os;
    }

    struct sched : basic_session {
        maud_id timer_;
        tmqueue queue_;
        void
        checkexpired(const timeval& tv)
        {
            time_t now(tv.tv_sec);

            while (!queue_.empty() && queue_.begin()->first <= now) {

                tmeventptr ptr(queue_.begin()->second);
                queue_.erase(queue_.begin());
                maud_post("schedclient", ptr->name_.c_str(), 0);
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
                else
                    timer_ = maud::settimer(ms, 0);
                aug_info("next expiry in %d ms", ms);
            } else if (-1 != timer_) {
                canceltimer(timer_);
                timer_ = -1;
            }
        }
        void
        delevent(const map<string, string>& params)
        {
            maud_id id(getvalue<maud_id>(params, "id"));

            aug_info("deleting event: id=[%d]", (int)id);
            eraseevent(queue_, id);

            timeval tv;
            gettimeofday(tv);
            settimer(tv);
        }
        void
        putevent(const map<string, string>& params)
        {
            maud_id id(getvalue<maud_id>(params, "id"));

            if (id) {
                aug_info("updating event: id=[%d]", id);
                eraseevent(queue_, id);
            } else {
                aug_info("inserting new event");
                id = aug_nextid();
            }

            string name(getvalue<string>(params, "name"));
            string spec(getvalue<string>(params, "spec"));

            tmtz tz(TMUTC);
            if (getvalue<string>(params, "tz") == "local")
                tz = TMLOCAL;

            timeval tv;
            gettimeofday(tv);

            tmeventptr ptr(new tmevent(id, name, spec, tz));
            if (aug_strtmspec(&ptr->tmspec_, spec.c_str()))
                pushevent(queue_, tv.tv_sec, ptr);

            settimer(tv);
        }
        void
        respond(const char* from, const char* type, const string& urlencoded)
        {
            map<string, string> params;
            urlunpack(urlencoded.begin(), urlencoded.end(),
                      inserter(params, params.begin()));

            if (0 == strcmp(type, "http.sched.delevent")) {
                delevent(params);
            } else if (0 == strcmp(type, "http.sched.putevent")) {
                putevent(params);
            } else if (0 != strcmp(type, "http.sched.events"))
                return;

            unsigned offset(getvalue<unsigned>(params, "offset"));
            unsigned max_(getvalue<unsigned>(params, "max"));

            stringstream ss;
            toxml(ss, queue_, offset, max_);
            scoped_blob<stringob> blob(ss.str());
            dispatch(from, "result", blob.base());
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
        do_event(const char* from, const char* type, struct aug_object_* ob)
        {
            aug_info("event [%s] triggered", type);

            smartob<aug_blob> blob(object_cast<aug_blob>(obptr(ob)));
            if (null == blob)
                return;

            size_t size;
            const char* encoded
                (static_cast<const char*>(blobdata(blob, &size)));
            if (!encoded)
                return;

            map<string, string> fields;
            urlunpack(encoded, encoded + size,
                      inserter(fields, fields.begin()));

            map<string, string>::iterator it(fields.find("content"));
            if (it != fields.end() && !it->second.empty())
                it->second = filterbase64(it->second.data(),
                                          it->second.size(), AUG_DECODE64);

            map<string, string>::const_iterator jt(fields.begin()),
                end(fields.end());
            for (; jt != end; ++jt)
                writelog(MAUD_LOGINFO, "%s=%s", jt->first.c_str(),
                         jt->second.c_str());

            respond(from, type, fields["content"]);
        }
        void
        do_expire(const handle& timer, unsigned& ms)
        {
            timeval tv;
            gettimeofday(tv);

            checkexpired(tv);

            ms = timerms(queue_, tv);
            aug_info("next expiry in %d ms", ms);
        }
        ~sched() AUG_NOTHROW
        {
        }
        sched()
            : timer_(-1)
        {
        }
        static session_base*
        create(const char* sname)
        {
            return new sched();
        }
    };

    typedef basic_module<basic_factory<sched> > module;
}

MAUD_ENTRYPOINTS(module::init, module::term)
