/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define MOD_BUILD
#include "file.hpp"

#include "augmodpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augmarpp.hpp"
#include "augsyspp.hpp"

#include <map>
#include <fstream>
#include <sstream>

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    string css_;

    // close/sessid.

    typedef map<mod_id, pair<string, bool> > active;
    active active_;

    void
    loadcss()
    {
        const char* css(mod::getenv("session.http.css"));
        if (css) {
            aug::chdir(mod::getenv("rundir"));
            ifstream fs(css);
            stringstream ss;
            ss << fs.rdbuf();
            css_ = ss.str();
        } else
            css_.clear();
    }

    const char*
    nonce(mod_id id, const string& addr, aug_md5base64_t base64)
    {
        pid_t pid(getpid());

        timeval tv;
        clockptr clock(getclock(aug_tlx));
        gettimeofday(clock, tv);

        long rand(aug_rand());
        const char* salt(mod::getenv("session.http.salt"));

        aug_md5context md5ctx;
        unsigned char digest[16];

        aug_initmd5(&md5ctx);
        aug_appendmd5(&md5ctx,
                      reinterpret_cast<const unsigned char*>(&pid),
                      sizeof(pid));
        aug_appendmd5(&md5ctx,
                      reinterpret_cast<const unsigned char*>(&id),
                      sizeof(id));
        aug_appendmd5(&md5ctx,
                      reinterpret_cast<const unsigned char*>(addr.data()),
                      static_cast<unsigned>(addr.size()));
        aug_appendmd5(&md5ctx,
                      reinterpret_cast<const unsigned char*>(&tv.tv_sec),
                      sizeof(tv.tv_sec));
        aug_appendmd5(&md5ctx,
                      reinterpret_cast<const unsigned char*>(&tv.tv_usec),
                      sizeof(tv.tv_usec));
        aug_appendmd5(&md5ctx,
                      reinterpret_cast<const unsigned char*>(&rand),
                      sizeof(rand));

        if (salt)
            aug_appendmd5(&md5ctx,
                          reinterpret_cast<const unsigned char*>(salt),
                          static_cast<unsigned>(strlen(salt)));

        aug_finishmd5(digest, &md5ctx);
        aug_md5base64(digest, base64);
        return base64;
    }

    void
    splitlist(map<string, string>& pairs, const string& encoded, char delim)
    {
        vector<string> toks(splitn(encoded.begin(), encoded.end(), delim));
        vector<string>::const_iterator it(toks.begin()), end(toks.end());
        for (; it != end; ++it) {
            string x, y;
            split2(it->begin(), it->end(), x, y, '=');
            trim(x);
            trim(y);
            trim(y, "\"");
            pairs[x] = y;
        }
    }

    string
    getsessid(mod_id id, const string& addr, const char* cookie)
    {
        aug_md5base64_t base64;

        if (cookie) {

            map<string, string> pairs;
            splitlist(pairs, cookie, ';');

            map<string, string>
                ::const_iterator it(pairs.find("X-Aug-Session"));

            // Have session cookie and associated map entry.

            if (it != pairs.end())
                return it->second;
        }

        // Theoretical possibility of collision.

        return nonce(id, addr, base64);
    }

    string
    utcdate()
    {
        char buf[64];
        time_t t;
        time(&t);
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
        return buf;
    }

    bool
    getpass(const string& user, const string& realm, string& digest)
    {
        const char* passwd(mod::getenv("session.http.passwd"));
        if (!passwd)
            return false;

        aug::chdir(mod::getenv("rundir"));
        ifstream fs(passwd);
        string line;
        while (getline(fs, line)) {

            trim(line);
            if (line.empty() || '#' == line[0])
                continue;

            vector<string> toks(splitn(line.begin(), line.end(), ':'));
            if (3 != toks.size())
                return false;

            if (toks[0] == user && toks[1] == realm) {
                digest = toks[2];
                return true;
            }
        }
        return false;
    }

    bool
    basicauth(const string& encoded, const string& realm)
    {
        string xy, x, y;
        xy = filterbase64(encoded.data(), encoded.size(), AUG_DECODE64);
        split2(xy.begin(), xy.end(), x, y, ':');

        aug_ctxinfo(aug_tlx, "basic user authentication [%s]", x.c_str());

        string digest;
        if (!getpass(x, realm, digest))
            return false;

        aug_md5base64_t base64;
        if (digest != aug_digestpass(x.c_str(), realm.c_str(), y.c_str(),
                                     base64))
            return false;
        return true;
    }

    bool
    digestauth(const string& list, const string& realm, const string& method)
    {
        map<string, string> pairs;
        splitlist(pairs, list, ',');

        aug_ctxinfo(aug_tlx, "digest user authentication [%s]",
                    pairs["username"].c_str());

        string digest;
        if (!getpass(pairs["username"], pairs["realm"], digest))
            return false;

        aug_md5base64_t ha1;
        strcpy(ha1, digest.c_str());
        aug_md5base64_t ha2 = "";
        aug_md5base64_t response;

        aug_digestresponse(ha1, pairs["nonce"].c_str(), pairs["nc"].c_str(),
                           pairs["cnonce"].c_str(), pairs["qop"].c_str(),
                           method.c_str(), pairs["uri"].c_str(), ha2,
                           response);

        if (pairs["response"] != response) {
            aug_ctxinfo(aug_tlx, "access denied: expected=[%s]", response);
            return false;
        }

        return true;
    }

    struct request {
        string method_, uri_, version_;
    };

    struct uri {
        string base_, path_, query_;
    };

    request&
    splitrequest(request& r, const string& s)
    {
        string::size_type first, last;
        if (string::npos == (first = s.find_first_of(' '))
            || string::npos == (last = s.find_last_of(' ')))
            throw http_error(400, "Bad Request");
        r.method_ = s.substr(0, first++);
        r.uri_ = s.substr(first, last - first);
        r.version_ = s.substr(last + 1);
        return r;
    }

    uri&
    splituri(uri& u, const string& s)
    {
        string::size_type pos;
        if (0 == s.compare(0, (pos = 7), "http://")
            || 0 == s.compare(0, (pos = 8), "https://")) {

            if (string::npos == (pos = s.find_first_of('/', pos))) {
                u.base_ = s;
                return u;
            }

            u.base_ = s.substr(0, pos++);
            u.path_ = s.substr(pos);

        } else
            u.path_ = s;

        if (string::npos != (pos = u.path_.find_last_of('?'))) {
            u.query_ = u.path_.substr(pos + 1);
            u.path_.erase(pos);
        }

        return u;
    }

    void
    setstatus(ostream& os, int code, const string& status)
    {
        os << "<html><head><title>" << code << "&nbsp;"
           << status << "</title>";
        if (!css_.empty())
            os << "<style type=\"text/css\">" << css_ << "</style>";
        os << "</head><body><h1>" << status << "</h1></body></html>";
    }

    marptr
    createstatus(int code, const string& status)
    {
        marptr mar(createmar(getmpool(aug_tlx)));
        putfieldp(mar, "Cache-Control", "no-cache");
        putfieldp(mar, "Content-Type", "text/html");

        omarstream os(mar);
        setstatus(os, code, status);
        return mar;
    }

    void
    respond(mod_id id, const string& sessid, int code, const string& status,
            marref mar, bool close)
    {
        blobptr blob(object_cast<aug_blob>(mar));
        const size_t size(getblobsize(blob));

        stringstream message;
        message << "HTTP/1.1 " << code << ' ' << status << "\r\n"
                << "Date: " << utcdate() << "\r\n"
                << "Set-Cookie: X-Aug-Session=" << sessid << "\r\n";

        // Inject custom headers.

        header header(mar);
        header::const_iterator it(header.begin()),
            end(header.end());
        for (; it != end; ++it) {

            unsigned size(0);
            const char* value(static_cast<
                              const char*>(header.getfield(it, size)));
            message << *it << ": ";
            message.write(value, size) << "\r\n";
        }

        message << "Content-Length: " << static_cast<unsigned>(size)
                << "\r\n\r\n";

        send(id, message.str().c_str(), message.str().size());
        sendv(id, blob.get());

        if (close) {
            aug_ctxinfo(aug_tlx, "closing");
            mod::shutdown(id, 0);
        }
    }

    void
    respond(mod_id id, const string& sessid, int code, const string& status,
            const string& mimetype, blobref blob, bool close)
    {
        const size_t size(getblobsize(blob));

        stringstream message;
        message << "HTTP/1.1 " << code << ' ' << status << "\r\n"
                << "Date: " << utcdate() << "\r\n"
                << "Set-Cookie: X-Aug-Session=" << sessid << "\r\n"
                << "Content-Type: " << mimetype << "\r\n"
                << "Content-Length: " << static_cast<unsigned>(size)
                << "\r\n\r\n";

        send(id, message.str().c_str(), message.str().size());
        sendv(id, blob.get());

        if (close) {
            aug_ctxinfo(aug_tlx, "closing");
            mod::shutdown(id, 0);
        }
    }

    void
    respond(mod_id id, const string& sessid, int code, const string& status,
            bool close)
    {
        marptr mar(createstatus(code, status));
        respond(id, sessid, code, status, mar, close);
    }

    struct session : marstore_base<session>, mpool_ops {
        const string& realm_;
        mod_id id_;
        string sessid_;
        const string name_;
        aug_md5base64_t nonce_;
        bool auth_;
        session(const string& realm, mod_id id, const string& name)
            : realm_(realm),
              id_(id),
              name_(name),
              auth_(false)
        {
            nonce(id_, name_, nonce_);
            aug_ctxinfo(aug_tlx, "nonce: [%s]", nonce_);
        }
        void
        put(const char* request, marref mar)
        {
            // TODO: externalise root directory path.

            static const char ROOT[] = "./htdocs";

            if (sessid_.empty()) {

                const void* value;
                getfieldp(mar, "Cookie", value);

                // Returned cookie may be null.

                sessid_ = getsessid(id_, name_,
                                    static_cast<const char*>(value));
            }

            const void* value;
            unsigned size(getfieldp(mar, "Connection", value));
            const bool close(value && size
                             && aug_strcasestr(static_cast<
                                               const char*>(value), "close"));

            active_[id_] = make_pair(sessid_, close);

            try {

                aug_ctxinfo(aug_tlx, "%s", request);

				struct request r;
                splitrequest(r, request);
                aug_ctxinfo(aug_tlx, "method: [%s]", r.method_.c_str());
                aug_ctxinfo(aug_tlx, "uri: [%s]", r.uri_.c_str());
                aug_ctxinfo(aug_tlx, "version: [%s]", r.version_.c_str());

                uri u;
                splituri(u, r.uri_);

                aug_ctxinfo(aug_tlx, "base: [%s]", u.base_.c_str());
                aug_ctxinfo(aug_tlx, "path: [%s]", u.path_.c_str());
                aug_ctxinfo(aug_tlx, "query: [%s]", u.query_.c_str());

                if (r.method_ == "GET") {

                    if (!u.query_.empty()) {

                        // Convert query to post.

                        putfieldp(mar, "Content-Type",
                                  "application/x-www-form-urlencoded");
                        setcontent(mar, u.query_);
                    }

                } else if (r.method_ != "POST")
                    throw http_error(501, "Not Implemented");

                // For each header field.

                header header(mar);
                header::const_iterator it(header.begin()),
                    end(header.end());
                for (; it != end; ++it) {

                    unsigned size(0);
                    const char* value
                        (static_cast<const char*>(header.getfield(it, size)));

                    aug_ctxinfo(aug_tlx, "%s: %s", *it, value);

                    if (0 == strcmp(*it, "Authorization")) {

                        // Authenticate.

                        string x, y;
                        split2(value, value + size, x, y, ' ');
                        if (x == "Basic")
                            auth_ = basicauth(y, realm_);
                        else if (x == "Digest")
                            auth_ = digestauth(y, realm_, r.method_);
                        else
                            throw http_error(501, "Not Implemented");
                    }
                }

                // Split path into series of nodes.

                u.path_ = urldecode(u.path_.begin(), u.path_.end());
                vector<string> nodes(splitpath(u.path_));

                if (!auth_) {

                    // Request authentication.

                    stringstream ss;
                    ss << "Digest realm=\"" << realm_
                       << "\", qop=\"auth,auth-int\", nonce=\""
                       << nonce_ << "\"";

                    marptr resp(createstatus(401, "Unauthorized"));
                    putfieldp(resp, "WWW-Authenticate", ss.str());
                    respond(id_, sessid_, 401, "Unauthorized", resp, close);

                } else if (nodes.empty() || nodes[0] != "service") {

                    // Static file system content.

                    string path(joinpath(ROOT, nodes));
                    aug_ctxinfo(aug_tlx, "path [%s]", path.c_str());

                    blobptr blob(getfile(path.c_str()));
                    respond(id_, sessid_, 200, "OK", mimetype(path), blob,
                            close);

                } else {

                    string path(jointype(nodes));
                    post(id_, "http-request", path.c_str(), mar.base());
                }

            } catch (const http_error& e) {
                aug_ctxerror(aug_tlx, "%d: %s", e.code(), e.what());
                respond(id_, sessid_, e.code(), e.what(), close);
            } catch (const exception&) {
                respond(id_, sessid_, 500, "Internal Server Error", close);
                throw;
            }
        }
        aug_result
        delmar_(const char* request) AUG_NOTHROW
        {
            // TODO: implement pool.

            aug_ctxinfo(aug_tlx, "delete mar [%s]", request);
            return AUG_SUCCESS;
        }
        aug_mar_*
        getmar_(const char* request) AUG_NOTHROW
        {
            // TODO: implement pool.

            aug_ctxinfo(aug_tlx, "get mar [%s]", request);
            return aug_createmar(getmpool(aug_tlx).get());
        }
        aug_result
        putmar_(const char* request, aug_mar_* mar) AUG_NOTHROW
        {
            // TODO: implement pool.

            try {
                aug_ctxinfo(aug_tlx, "put mar [%s]", request);
                put(request, mar);
                return AUG_SUCCESS;
            } AUG_SETERRINFOCATCH;
            return AUG_FAILERROR;
        }
    };

    struct http : basic_session, mpool_ops {

        string realm_;
        bool
        do_start(const char* sname)
        {
            aug_ctxinfo(aug_tlx, "starting...");
            const char* serv(mod::getenv("session.http.serv"));
            const char* realm(mod::getenv("session.http.realm"));
            if (!serv || !realm)
                return false;

            realm_ = realm;
            tcplisten("0.0.0.0", serv);
            do_reconf();
            return true;
        }
        void
        do_reconf()
        {
            loadcss();
            loadmimetypes();
        }
        void
        do_event(mod_id id, const char* from, const char* type,
                 aug_object_* ob)
        {
            smartob<aug_mar> mar(object_cast<aug_mar>(obptr(ob)));
            if (null != mar) {
                active::const_iterator it(active_.find(id));
                if (it != active_.end())
                    respond(id, it->second.first, 200, "OK", mar,
                            it->second.second);
            }
        }
        void
        do_closed(const handle& sock)
        {
            aug_ctxinfo(aug_tlx, "closed");
            if (sock.user()) {
                auto_ptr<marparser> parser(sock.user<marparser>());
                finishmar(*parser);
            }

            // Remove association between session and connection.

            active_.erase(sock.id());
        }
        bool
        do_accepted(handle& sock, const char* name)
        {
            marstoreptr sess(session::attach(new (tlx) session
                                             (realm_, sock.id(), name)));
            auto_ptr<marparser> parser(new (tlx) marparser
                                       (getmpool(aug_tlx), sess));

            sock.setuser(parser.get());
            setrwtimer(sock, 30000, MOD_TIMRD);
            parser.release();
            return true;
        }
        void
        do_recv(const handle& sock, const void* buf, size_t len)
        {
            marparser& parser(*sock.user<marparser>());
            try {
                appendmar(parser, static_cast<const char*>(buf),
                          static_cast<unsigned>(len));
            } catch (...) {
                mod::shutdown(sock, 1);
                throw;
            }
        }
        void
        do_rdexpire(const handle& sock, unsigned& ms)
        {
            aug_ctxinfo(aug_tlx, "no data received for 30 seconds");
            shutdown(sock, 0);
        }
        ~http() AUG_NOTHROW
        {
        }
        static session_base*
        create(const char* sname)
        {
            return new (tlx) http();
        }
    };

    typedef basic_module<basic_factory<http> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
