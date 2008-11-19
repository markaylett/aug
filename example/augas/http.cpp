/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define MOD_BUILD
#include "augmodpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augmarpp.hpp"
#include "augsyspp.hpp"

#include <fstream>
#include <map>
#include <sstream>

#include <sys/stat.h>

#if !defined(_WIN32)
# define _stat stat
# define _S_ISDIR S_ISDIR
# define _S_ISREG S_ISREG
# define _S_IREAD S_IROTH
#else // _WIN32
# define lstat _stat
# if !defined(_S_ISDIR)
#  define _S_ISDIR(mode) (((mode) & _S_IFDIR) == _S_IFDIR)
# endif // !_S_ISDIR
# if !defined(_S_ISREG)
#  define _S_ISREG(mode) (((mode) & _S_IFREG) == _S_IFREG)
# endif // !_S_ISREG
#endif // _WIN32

using namespace aug;
using namespace mod;
using namespace std;

namespace {

    string css_;
    map<string, string> mimetypes_;
    map<string, time_t> sessids_;

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

    aug_result
    confcb_(void* arg, const char* name, const char* value)
    {
        mimetypes_[name] = value;
        return AUG_SUCCESS;
    }

    void
    loadmimetypes()
    {
        mimetypes_.clear();

        // Add some basic types.

        mimetypes_["css"] = "text/css";
        mimetypes_["gif"] = "image/gif";
        mimetypes_["html"] = "text/html";
        mimetypes_["htm"] = "text/html";
        mimetypes_["jpeg"] = "image/jpeg";
        mimetypes_["jpg"] = "image/jpeg";
        mimetypes_["js"] = "application/x-javascript";
        mimetypes_["png"] = "image/png";
        mimetypes_["tif"] = "image/tiff";
        mimetypes_["tiff"] = "image/tiff";
        mimetypes_["txt"] = "text/plain";
        mimetypes_["xml"] = "text/xml";

        const char* mimetypes(mod::getenv("session.http.mimetypes"));
        if (mimetypes) {
            aug::chdir(mod::getenv("rundir"));
            readconf(mimetypes, confcb<confcb_>, null);
        }
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
        aug_appendmd5(&md5ctx, (unsigned char*)&pid, sizeof(pid));
        aug_appendmd5(&md5ctx, (unsigned char*)&id, sizeof(id));
        aug_appendmd5(&md5ctx, (unsigned char*)addr.data(),
                      (unsigned)addr.size());
        aug_appendmd5(&md5ctx, (unsigned char*)&tv.tv_sec, sizeof(tv.tv_sec));
        aug_appendmd5(&md5ctx, (unsigned char*)&tv.tv_usec,
                      sizeof(tv.tv_usec));
        aug_appendmd5(&md5ctx, (unsigned char*)&rand, sizeof(rand));
        if (salt)
            aug_appendmd5(&md5ctx, (unsigned char*)salt,
                          (unsigned)strlen(salt));
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

            map<string, string>::const_iterator it(pairs.find("AUGSESSID"));

            // Have session cookie and associated map entry.

            if (it != pairs.end()
                && sessids_.find(it->second) != sessids_.end())
                return it->second;
        }

        // Create new session id.

        string sessid;
        do {

            sessid = nonce(id, addr, base64);

            // Loop while duplicate.

        } while (sessids_.find(sessid) != sessids_.end());

        // Note on duplicates.

        sessids_[sessid] = 0;
        return sessid;
    }

    void
    sethead(ostream& os, const string& title)
    {
        os << "<head><title>" << title << "</title>";
        if (!css_.empty())
            os << "<style type=\"text/css\">" << css_ << "</style>";
        os << "</head>";
    }

    string
    mimetype(const string& path)
    {
        string type("text/plain");

        string::size_type pos(path.find_last_of('.'));
        if (pos != string::npos) {
            string ext(path.substr(pos + 1));
            map<string, string>::const_iterator it(mimetypes_.find(ext));
            if (it != mimetypes_.end())
                type = it->second;
        }

        return type;
    }

    class http_error : public domain_error {
        const int status_;
    public:
        http_error(int status, const string& title)
            : domain_error(title),
              status_(status)
        {
        }
        int
        status() const
        {
            return status_;
        }
    };

    bool
    lstat(const char* path, struct _stat& sb)
    {
        // lstat() is used so that the link, rather than the file it
        // references, is stat()-ed.

		if (-1 == ::lstat(path, &sb)) {
            if (ENOENT != errno)
                throw posix_error(__FILE__, __LINE__, errno);
            return false;
        }
        return true;
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

        aug_ctxinfo(aug_tlx, "authenticating user [%s]", x.c_str());

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

        aug_ctxinfo(aug_tlx, "authenticating user [%s]",
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

    class nodes {
        vector<string>* xs_;
    public:
        explicit
        nodes(vector<string>& xs)
            : xs_(&xs)
        {
        }
        void
        operator ()(string& x)
        {
            if (x == "..") {
                if (xs_->empty())
                    throw http_error(403, "Forbidden");
                xs_->pop_back();
            } else if (!x.empty() && x != ".")
                xs_->push_back(x);
        }
    };

    vector<string>
    splitpath(const string& path)
    {
        vector<string> v;
        splitn(path.begin(), path.end(), '/', nodes(v));
        return v;
    }

    string
    joinpath(const char* root, const vector<string>& nodes)
    {
        string path(root);
        struct _stat sb;

        vector<string>::const_iterator it(nodes.begin()), end(nodes.end());
        for (; it != end; ++it) {
            path += '/';
            path += *it;

            if (!lstat(path.c_str(), sb))
                throw http_error(404, "Not Found");

            // Do not follow symbolic links.

            if (!_S_ISDIR(sb.st_mode) && !_S_ISREG(sb.st_mode))
                throw http_error(403, "Forbidden");
        }

        // Use index.html if directory.

        if (nodes.empty() || _S_ISDIR(sb.st_mode)) {
            path += "/index.html";
            if (!lstat(path.c_str(), sb))
                throw http_error(404, "Not Found");
        }

        // Must be regular, world-readable file.

        if (!_S_ISREG(sb.st_mode) || !(sb.st_mode & _S_IREAD))
            throw http_error(403, "Forbidden");

        return path;
    }

    string
    jointype(const vector<string>& nodes)
    {
        string type;
        vector<string>::const_iterator it(nodes.begin()), end(nodes.end());
        for (; it != end; ++it) {
            if (!type.empty())
                type += '.';
            type += *it;
        }
        return type;
    }

    class filecontent : public ref_base {
        blob<filecontent> blob_;
        autofd sfd_;
        mmap mmap_;
        ~filecontent() AUG_NOTHROW
        {
        }
        filecontent(mpoolref mpool, const char* path)
            : sfd_(aug::open(path, O_RDONLY)),
              mmap_(mpool, sfd_, 0, 0, AUG_MMAPRD)
        {
            blob_.reset(this);
        }
    public:
        smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_blob>(id))
                return object_retain<aug_object>(blob_);
            return null;
        }
        const void*
        getblobdata_(size_t* size) AUG_NOTHROW
        {
            if (size)
                *size = mmap_.len();
            return mmap_.addr();
        }
        size_t
        getblobsize_() AUG_NOTHROW
        {
            return mmap_.len();
        }
        static smartob<aug_blob>
        create(const char* path)
        {
            filecontent* ptr = new filecontent(getmpool(aug_tlx), path);
            return object_attach<aug_blob>(ptr->blob_);
        }
    };

    typedef vector<pair<string, string> > fields;

    void
    sendfile(mod_id id, const string& sessid, const string& path)
    {
        smartob<aug_blob> blob(filecontent::create(path.c_str()));
        size_t size(getblobsize(blob));

        stringstream header;
        header << "HTTP/1.1 200 OK\r\n"
               << "Date: " << utcdate() << "\r\n"
               << "Set-Cookie: AUGSESSID=" << sessid << "\r\n"
               << "Content-Type: " << mimetype(path) << "\r\n"
               << "Content-Length: " << (unsigned)size << "\r\n"
               << "\r\n";

        send(id, header.str().c_str(), header.str().size());
        sendv(id, blob.get());
    }

    vector<string> results_;

    void
    sendresult(mod_id id, const string& sessid)
    {
        stringstream content;
        content << "<result>";
        vector<string>::const_iterator it(results_.begin()),
            end(results_.end());
        for (; it != end; ++it)
            content << *it;
        content << "</result>";

        stringstream message;
        message << "HTTP/1.1 200 OK\r\n"
                << "Cache-Control: no-cache\r\n"
                << "Date: " << utcdate() << "\r\n"
                << "Set-Cookie: AUGSESSID=" << sessid << "\r\n"
                << "Content-Type: text/xml\r\n"
                << "Content-Length: " << (unsigned)content.str().size()
                << "\r\n\r\n"
                << content.rdbuf();

        send(id, message.str().c_str(), message.str().size());
    }

    void
    sendstatus(mod_id id, const string& sessid, int status,
               const string& title, const fields& fs = fields())
    {
        stringstream content;
        content << "<html>";
        sethead(content, status + "&nbsp;" + title);
        content << "<body><h1>" << title << "</h1></body></html>";

        stringstream message;
        message << "HTTP/1.1 " << status << ' ' << title << "\r\n"
                << "Cache-Control: no-cache\r\n"
                << "Date: " << utcdate() << "\r\n"
                << "Set-Cookie: AUGSESSID=" << sessid << "\r\n";

        fields::const_iterator it(fs.begin()), end(fs.end());
        for (; it != end; ++it)
            message << it->first << ": " << it->second << "\r\n";

        message << "Content-Type: text/html\r\n"
                << "Content-Length: " << (unsigned)content.str().size()
                << "\r\n\r\n"
                << content.rdbuf();

        send(id, message.str().c_str(), message.str().size());
    }

    struct session : marpool_base<session> {
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
        put(const char* initial, aug_mar_t mar)
        {
            static const char ROOT[] = "./htdocs";

            if (sessid_.empty())
                sessid_ = getsessid(id_, name_, static_cast<
                                    const char*>(getfield(mar, "Cookie")));

            try {

                aug_ctxinfo(aug_tlx, "%s", initial);

                request r;
                splitrequest(r, initial);
                aug_ctxinfo(aug_tlx, "method: [%s]", r.method_.c_str());
                aug_ctxinfo(aug_tlx, "uri: [%s]", r.uri_.c_str());
                aug_ctxinfo(aug_tlx, "version: [%s]", r.version_.c_str());

                uri u;
                splituri(u, r.uri_);

                aug_ctxinfo(aug_tlx, "base: [%s]", u.base_.c_str());
                aug_ctxinfo(aug_tlx, "path: [%s]", u.path_.c_str());
                aug_ctxinfo(aug_tlx, "query: [%s]", u.query_.c_str());

                string contenttype, content;

                if (r.method_ == "POST") {

                    contenttype = static_cast<const char*>
                        (getfield(mar, "Content-Type"));

                    unsigned size;
                    const void* ptr(aug::getcontent(mar, size));
                    content = filterbase64(static_cast<const char*>(ptr),
                                           size, AUG_ENCODE64);

                } else if (r.method_ == "GET") {

                    if (!u.query_.empty()) {
                        contenttype = "application/x-www-form-urlencoded";
                        content = filterbase64(u.query_.data(),
                                               u.query_.size(), AUG_ENCODE64);
                    }

                } else
                    throw http_error(501, "Not Implemented");

                header header(mar);
                header::const_iterator it(header.begin()),
                    end(header.end());
                for (; it != end; ++it) {

                    unsigned size;
                    const char* value
                        (static_cast<const char*>(header.getfield(it, size)));

                    aug_ctxinfo(aug_tlx, "%s: %s", *it, value);

                    if (0 == strcmp(*it, "Authorization")) {

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

                u.path_ = urldecode(u.path_.begin(), u.path_.end());
                vector<string> nodes(splitpath(u.path_));

                if (!auth_) {

                    stringstream ss;
                    ss << "Digest realm=\"" << realm_
                       << "\", qop=\"auth,auth-int\", nonce=\""
                       << nonce_ << "\"";

                    fields fs;
                    fs.push_back(make_pair("WWW-Authenticate", ss.str()));
                    sendstatus(id_, sessid_, 401, "Unauthorized", fs);

                } else if (!nodes.empty() && nodes[0] == "service") {

                    nodes[0] = "http";
                    string type(jointype(nodes));

                    results_.clear();

                    if (type == "http.reconf") {

                        reconfall();
                        results_.push_back("<message type=\"info\">"
                                           "re-configured</message>");

                    } else {

                        map<string, string> values;
                        values["contenttype"] = contenttype;
                        values["content"] = content;
                        values["sessid"] = sessid_;

                        // Dispatch is synchronous.

                        scoped_blob_wrapper<sblob> blob
                            (urlpack(values.begin(), values.end()));

                        dispatch("httpclient", type.c_str(), blob.base());
                    }

                    sendresult(id_, sessid_);

                } else {

                    string path(joinpath(ROOT, nodes));
                    aug_ctxinfo(aug_tlx, "path [%s]", path.c_str());
                    sendfile(id_, sessid_, path);
                }

            } catch (const http_error& e) {
                aug_ctxerror(aug_tlx, "%d: %s", e.status(), e.what());
                sendstatus(id_, sessid_, e.status(), e.what());
            } catch (const exception&) {
                sendstatus(id_, sessid_, 500, "Internal Server Error");
                throw;
            }

            unsigned size;
            const char* value(static_cast<const char*>
                              (getfield(mar, "Connection", size)));
            if (value && size && aug_strcasestr(value, "close")) {
                aug_ctxinfo(aug_tlx, "closing");
                mod::shutdown(id_, 0);
            }
        }
        aug_result
        delmar_(const char* initial) AUG_NOTHROW
        {
            return AUG_SUCCESS;
        }
        struct aug_mar_*
        getmar_(const char* initial) AUG_NOTHROW
        {
            return 0;
        }
        aug_result
        putmar_(const char* initial, struct aug_mar_* mar) AUG_NOTHROW
        {
            try {
                put(initial, mar);
                return AUG_SUCCESS;
            } AUG_SETERRINFOCATCH;
            return AUG_FAILERROR;
        }
    };

    struct http : basic_session {

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
        do_event(const char* from, const char* type, struct aug_object_* ob)
        {
            smartob<aug_blob> blob(object_cast<aug_blob>(obptr(ob)));
            if (null != blob) {
                size_t size;
                const void* data(getblobdata(blob, &size));
                if (data) {
                    string xml(static_cast<const char*>(data), size);
                    results_.push_back(xml);
                }
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
        }
        bool
        do_accepted(handle& sock, const char* name)
        {
            marpoolptr sess(session::attach(new session(realm_, sock.id(),
                                                        name)));
            auto_ptr<marparser> parser(new marparser(getmpool(aug_tlx),
                                                     sess));

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
            return new http();
        }
    };

    typedef basic_module<basic_factory<http> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
