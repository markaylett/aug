#include "augaspp.hpp"
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
#endif // _WIN32

using namespace aug;
using namespace augas;
using namespace std;

namespace {

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

        if (-1 == lstat(path, &sb)) {
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

    class filecontent {
        smartfd sfd_;
        mmap mmap_;
    public:
        typedef filecontent arg_type;
        explicit
        filecontent(const char* path)
            : sfd_(aug::open(path, O_RDONLY)),
              mmap_(sfd_, 0, 0, AUG_MMAPRD)
        {
        }
        void*
        addr() const
        {
            return mmap_.addr();
        }
        size_t
        size() const
        {
            return mmap_.len();
        }
        static void
        destroy(arg_type* arg)
        {
            delete arg;
        }
        static const void*
        buf(arg_type& arg, size_t& size)
        {
            size = arg.size();
            return arg.addr();
        }
        static const void*
        buf(arg_type& arg)
        {
            return arg.addr();
        }
    };

    typedef vector<pair<string, string> > fields;

    void
    sendfile(augas_id id, string path)
    {
        auto_ptr<filecontent> ptr(new filecontent(path.c_str()));

        stringstream header;
        header << "HTTP/1.1 200 OK\r\n"
               << "Date: " << utcdate() << "\r\n"
               << "Content-Type: text/html\r\n"
               << "Content-Length: " << ptr->size() << "\r\n"
               << "\r\n";

        send(id, header.str().c_str(), header.str().size());
        aug_var var;
        sendv(id, bindvar<filecontent>(var, *ptr));
        ptr.release();
    }

    typedef map<string, map<string, string> > pages;
    pages pages_;

    void
    sendhome(augas_id id)
    {
        stringstream content;
        content << "<html><head><title>AugAS</title></head>"
            "<body><h2>augas console</h2><table>"
            "<tr><th>service</th><th>status</th></tr>";

        pages::const_iterator it(pages_.begin()), end(pages_.end());
        for (; it != end; ++it) {

            map<string, string>::const_iterator home(it->second.find("home")),
                status(it->second.find("status"));

            if (home == it->second.end()
                && status == it->second.end())
                continue;

            content << "<tr><td>";

            if (home != it->second.end())
                content << "<a href=\"/services/" << it->first << "/home\">"
                        << it->first << "</a>";
            else
                content << it->first;

            content << "</td><td>";

            if (status != it->second.end())
                content << status->second;

            content << "</td></tr>";
        }

        content << "</table></body></html>";

        stringstream message;
        message << "HTTP/1.1 200 OK\r\n"
                << "Date: " << utcdate() << "\r\n"
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << content.str().size() << "\r\n"
                << "\r\n"
                << content.rdbuf();

        send(id, message.str().c_str(), message.str().size());
    }

    void
    sendpage(augas_id id, const string& service, const string& page)
    {
        pages::const_iterator it(pages_.find(service));
        if (it == pages_.end())
            throw http_error(404, "Not Found");

        map<string, string>::const_iterator jt(it->second.find(page));
        if (jt == it->second.end())
            throw http_error(404, "Not Found");

        stringstream content;
        content << "<html><head><title>"
                << service << "&nbsp;" << page
                << "</title></head><body><h2>" << service
                << "&nbsp;service</h2>" << jt->second
                << "</body></html>";

        stringstream message;
        message << "HTTP/1.1 200 OK\r\n"
                << "Date: " << utcdate() << "\r\n"
                << "Content-Type: text/html\r\n"
                << "Content-Length: " << content.str().size() << "\r\n"
                << "\r\n"
                << content.rdbuf();

        send(id, message.str().c_str(), message.str().size());
    }

    void
    sendstatus(augas_id id, int status, const string& title,
               const fields& fs = fields())
    {
        stringstream content;
        content << "<html><head><title>"
                << status << "&nbsp;" << title
                << "</title></head><body><h1>" << title
                << "</h1></body></html>";

        stringstream message;
        message << "HTTP/1.1 " << status << ' ' << title << "\r\n"
                << "Date: " << utcdate() << "\r\n";

        fields::const_iterator it(fs.begin()), end(fs.end());
        for (; it != end; ++it)
            message << it->first << ": " << it->second << "\r\n";

        message << "Content-Type: text/html\r\n"
                << "Content-Length: " << content.str().size() << "\r\n"
                << "\r\n"
                << content.rdbuf();

        send(id, message.str().c_str(), message.str().size());
    }

    struct handler : basic_marhandler {
        static void
        message(const aug_var& var, const char* initial, aug_mar_t mar)
        {
            static const char ROOT[] = ".";

            augas_id id(aug_ptoi(var.arg_));

            try {

                aug_info("%s", initial);

                request r;
                splitrequest(r, initial);
                aug_info("method: [%s]", r.method_.c_str());
                aug_info("uri: [%s]", r.uri_.c_str());
                aug_info("version: [%s]", r.version_.c_str());

                uri u;
                splituri(u, r.uri_);

                if (r.method_ == "POST") {
                    unsigned size;
                    const char* query(static_cast<
                                      const char*>(content(mar, size)));
                    u.query_ = string(query, size);
                } else if (r.method_ != "GET")
                    throw http_error(501, "Not Implemented");

                aug_info("base: [%s]", u.base_.c_str());
                aug_info("path: [%s]", u.path_.c_str());
                aug_info("query: [%s]", u.query_.c_str());

                header header(mar);
                header::const_iterator it(header.begin()),
                    end(header.end());
                for (; it != end; ++it)
                    aug_info("%s: %s", *it, header.getfield(it));

                u.path_ = urldecode(u.path_.begin(), u.path_.end());

                vector<string> nodes(splitpath(u.path_));
                if (nodes.empty())
                    sendhome(id);
                else {

                    if (nodes[0] == "services") {

                        if (2 == nodes.size()) {

                            dispatch(nodes[1].c_str(),
                                     "application/x-www-form-urlencoded",
                                     u.query_.c_str(), u.query_.size());

                            const char* value
                                (static_cast<
                                 const char*>(getfield(mar, "Host")));
                            fields fs;
                            fs.push_back
                                (make_pair("Location", string("http://")
                                           .append(value)));
                            sendstatus(id, 303, "See Other", fs);

                        } else if (3 == nodes.size())
                            sendpage(id, nodes[1], nodes[2]);
                        else
                            throw http_error(404, "Not Found");

                    } else {

                        string path(joinpath(ROOT, nodes));
                        aug_info("path [%s]", path.c_str());
                        sendfile(id, path);
                    }
                }

            } catch (const http_error& e) {
                aug_error("%d: %s", e.status(), e.what());
                sendstatus(id, e.status(), e.what());
            } catch (const exception& e) {
                sendstatus(id, 500, "Internal Server Error");
                throw;
            }

            unsigned size;
            const char* value(static_cast<const char*>
                              (getfield(mar, "Connection", size)));
            if (value && size && aug_strcasestr(value, "close")) {
                aug_info("closing");
                shutdown(id);
            }
        }
    };

    struct httpserv : basic_serv {
        bool
        do_start(const char* sname)
        {
            aug_info("starting...");
            const char* serv = augas::getenv("service.http.serv");
            if (!serv)
                return false;

            tcplisten("0.0.0.0", serv);
            return true;
        }
        void
        do_event(const char* from, const char* type, const void* user,
                 size_t size)
        {
            pages::iterator it(pages_.find(from));
            if (it == pages_.end())
                it = pages_.insert(make_pair(from, map<string, string>()))
                    .first;

            string page(static_cast<const char*>(user), size);

            pair<map<string, string>::iterator,
                bool> xy(it->second.insert(make_pair(type, page)));
            if (!xy.second)
                xy.first->second = page;
        }
        void
        do_closed(const object& sock)
        {
            aug_info("closed");
            if (sock.user()) {
                auto_ptr<marparser> parser(sock.user<marparser>());
                endmar(*parser);
            }
        }
        void
        do_teardown(const object& sock)
        {
            shutdown(sock);
        }
        bool
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            aug_var var = { 0, aug_itop(sock.id()) };
            sock.setuser(new marparser(0, marhandler<handler>(), var));
            setrwtimer(sock, 30000, AUGAS_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const void* buf, size_t size)
        {
            marparser& parser(*sock.user<marparser>());
            try {
                parsemar(parser, static_cast<const char*>(buf),
                         static_cast<unsigned>(size));
            } catch (...) {
                shutdown(sock);
                throw;
            }
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
            aug_info("no data received for 30 seconds");
            shutdown(sock);
        }
        static serv_base*
        create(const char* sname)
        {
            return new httpserv();
        }
    };

    typedef basic_module<basic_factory<httpserv> > module;
}

AUGAS_MODULE(module::init, module::term)
