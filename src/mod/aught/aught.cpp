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
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "file.hpp"

#include "exception.hpp"
#include "options.hpp"

#include "augnetpp/base64.hpp"
#include "augnetpp/mar.hpp"
#include "augutilpp/string.hpp"
#include "augmarpp/header.hpp"
#include "augmarpp/mar.hpp"
#include "augmarpp/stream.hpp"

#include "augmodpp.hpp"

#include "augnet/auth.h"
#include "augutil/md5.h"
#include "augutil/pwd.h"
#include "augsys/unistd.h"
#include "augsys/utility.h"
#include "augctx/base.h"
#include "augext/clock.h"

#include <map>
#include <sstream>

#include <time.h>

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    options options_;

    // close/sessid.

    typedef map<mod_id, pair<string, bool> > active;
    active active_;

    const char*
    nonce(mod_id id, const string& addr, aug_md5base64_t base64)
    {
        pid_t pid(getpid());

        aug_timeval tv;
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
    basicauth(const string& encoded, const string& realm)
    {
        string xy, x, y;
        xy = filterbase64(encoded.data(), encoded.size(), AUG_DECODE64);
        split2(xy.begin(), xy.end(), x, y, ':');

        aug_ctxinfo(aug_tlx, "basic user authentication [%s]", x.c_str());

        string digest;
        if (!options_.passwd(x, realm, digest))
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
        if (!options_.passwd(pairs["username"], pairs["realm"], digest))
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
        if (!options_.css().empty())
            os << "<style type=\"text/css\">" << options_.css() << "</style>";
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

            // Skip Content-Type - specified in blob.

            if (0 != strcmp(*it, "Content-Type")) {
                message << *it << ": ";
                message.write(value, size) << "\r\n";
            }
        }

        message << "Content-Type: " << getblobtype(blob) << "\r\n"
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
            blobref blob, bool close)
    {
        const size_t size(getblobsize(blob));

        // Assume no-cache for blobs.

        stringstream message;
        message << "HTTP/1.1 " << code << ' ' << status << "\r\n"
                << "Date: " << utcdate() << "\r\n"
                << "Set-Cookie: X-Aug-Session=" << sessid << "\r\n"
                   "Cache-Control: no-cache\r\n"
                   "Content-Type: " << getblobtype(blob) << "\r\n"
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

    class store : public marstore_base<store>, public mpool_ops {
        const string& realm_;
        mod_id id_;
        string sessid_;
        const string name_;
        aug_md5base64_t nonce_;
        bool auth_;
        store(const string& realm, mod_id id, const string& name)
            : realm_(realm),
              id_(id),
              name_(name),
              auth_(false)
        {
            nonce(id_, name_, nonce_);
            aug_ctxinfo(aug_tlx, "nonce: [%s]", nonce_);
        }
    public:
        ~store() MOD_NOTHROW
        {
            // Deleted from base.
        }
        void
        put(const char* request, marref mar)
        {
            // TODO: externalise root directory path.

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

                const void* override;
                getfieldp(mar, "X-HTTP-Method-Override", override);

                // Update method if it has been overridden.

                if (override) {
                    r.method_ = static_cast<const char*>(override);
                    aug_ctxinfo(aug_tlx, "method override: [%s]",
                                r.method_.c_str());
                }

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

                u.path_ = decodeurl(u.path_.begin(), u.path_.end());
                path nodes;
                try {
                    nodes = splitpath(u.path_);
                } catch (const underflow_error&) {

                    // Attempt to access root directory's parent.

                    throw http_error(403, "Forbidden");
                }

                if (!auth_) {

                    // Request authentication.

                    stringstream ss;
                    ss << "Digest realm=\"" << realm_
                       << "\", qop=\"auth,auth-int\", nonce=\""
                       << nonce_ << "\"";

                    marptr resp(createstatus(401, "Unauthorized"));
                    putfieldp(resp, "WWW-Authenticate", ss.str());
                    respond(id_, sessid_, 401, "Unauthorized", resp, close);

                    return;
                }

                string service(jointype(nodes));
                if (options_.service(service)) {

                    if (!u.query_.empty() && r.method_ != "POST") {

                        // Set content to be url-encoded query.

                        // Note: this is not done for the post method to avoid
                        // overwriting existing content.

                        putfieldp(mar, "Content-Type",
                                  "application/x-www-form-urlencoded");
                        setcontent(mar, u.query_);
                    }

                    // Ensure that method header is always set to facilitate
                    // REST-ful interfaces.

                    putfieldp(mar, "X-HTTP-Method-Override",
                              r.method_.c_str());

                    post("http-request", service.c_str(), id_, mar.base());

                } else if (r.method_ == "GET") {

                    // Static file system content.

                    string path(joinpath(nodes));
                    aug_ctxinfo(aug_tlx, "path [%s]", path.c_str());

                    blobptr blob(getfile(options_.mimetype(path),
                                         path.c_str()));
                    respond(id_, sessid_, 200, "OK", blob, close);

                } else if (r.method_ != "POST")
                    throw http_error(501, "Not Implemented");

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
        static marstoreptr
        create(const string& realm, mod_id id, const string& name)
        {
            return attach(new (tlx) store(realm, id, name));
        }
    };

    class parser : public boxptr_base<parser>, public mpool_ops {
        marparser impl_;
        explicit
        parser(mpoolref mpool, marstoreref marstore, unsigned size)
            : impl_(mpool, marstore, size)
        {
        }
    public:
        ~parser() MOD_NOTHROW
        {
            // Deleted from base.
        }
        void*
        unboxptr_() AUG_NOTHROW
        {
            return &impl_;
        }
        static boxptrptr
        create(mpoolref mpool, marstoreref marstore, unsigned size = 0)
        {
            return attach(new (tlx) parser(mpool, marstore, size));
        }
    };

    class http : public basic_session<http>, public mpool_ops {
        const string sname_;
        string realm_;

        explicit
        http(const string& sname)
            : sname_(sname)
        {
        }
    public:
        ~http() MOD_NOTHROW
        {
            // Deleted from base.
        }
        mod_bool
        start()
        {
            aug_ctxinfo(aug_tlx, "starting...");

            const char* realm(mod::getenv("session.http.realm", "aug"));
            if (!realm)
                return MOD_FALSE;

            const char* serv(mod::getenv("session.http.serv", "8080"));

            writelog(MOD_LOGINFO, "serv: %s", serv);
            writelog(MOD_LOGINFO, "realm: %s", realm);

            realm_ = realm;
            tcplisten("0.0.0.0", serv);

            const char* sslctx(mod::getenv("session.http.sslcontext"));
            if (sslctx) {

                const char* sslserv(mod::getenv("session.http.sslserv",
                                                "8443"));

                writelog(MOD_LOGINFO, "sslserv: %s", sslserv);
                writelog(MOD_LOGINFO, "sslcontext: %s", sslctx);

                tcplisten("0.0.0.0", sslserv, sslctx);
            }

            reconf();
            return MOD_TRUE;
        }
        void
        stop()
        {
        }
        void
        reconf()
        {
            options_.load();
        }
        void
        event(const char* from, const char* type, mod_id id, objectref ob)
        {
            active::const_iterator it(active_.find(id));
            if (it != active_.end()) {

                marptr mar(object_cast<aug_mar>(ob));
                if (null != mar) {
                    respond(id, it->second.first, 200, "OK", mar,
                            it->second.second);
                } else {

                    blobptr blob(object_cast<aug_blob>(ob));
                    if (null != blob)
                        respond(id, it->second.first, 200, "OK", blob,
                                it->second.second);
                }
            }
        }
        void
        closed(mod_handle& sock)
        {
            aug_ctxinfo(aug_tlx, "closed");
            marparser* parser(obtop<marparser>(sock.ob_));
            if (parser)
                finishmar(*parser);

            // Remove association between session and connection.

            aug_assign(sock.ob_, 0);
            active_.erase(sock.id_);
        }
        void
        teardown(mod_handle& sock)
        {
            mod::shutdown(sock, 0);
        }
        mod_bool
        accepted(mod_handle& sock, const char* name)
        {
            marstoreptr sp(store::create(realm_, sock.id_, name));
            boxptrptr bp(parser::create(getmpool(aug_tlx), sp));
            aug_assign(sock.ob_, bp.base());

            setrwtimer(sock, 30000, MOD_TIMRD);
            return MOD_TRUE;
        }
        void
        connected(mod_handle& sock, const char* name)
        {
        }
        mod_bool
        auth(mod_handle& sock, const char* subject, const char* issuer)
        {
            return MOD_TRUE;
        }
        void
        recv(mod_handle& sock, const void* buf, size_t len)
        {
            marparser* parser(obtop<marparser>(sock.ob_));
            try {
                appendmar(*parser, static_cast<const char*>(buf),
                          static_cast<unsigned>(len));
            } catch (...) {
                mod::shutdown(sock, 1);
                throw;
            }
        }
        void
        error(mod_handle& sock, const char* desc)
        {
        }
        void
        rdexpire(mod_handle& sock, unsigned& ms)
        {
            aug_ctxinfo(aug_tlx, "no data received for 30 seconds");
            shutdown(sock, 0);
        }
        void
        wrexpire(mod_handle& sock, unsigned& ms)
        {
        }
        void
        expire(mod_handle& timer, unsigned& ms)
        {
        }
        static sessionptr
        create(const char* sname)
        {
            return attach(new (tlx) http(sname));
        }
    };

    typedef basic_module<basic_factory<http> > module;
}

MOD_ENTRYPOINTS(module::init, module::term, module::create)
