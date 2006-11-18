/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MANAGER_HPP
#define AUGAS_MANAGER_HPP

#include "augas/conn.hpp"

#include <map>

namespace augas {

    class options;

    class manager {

        typedef std::map<std::string, moduleptr> modules;
        typedef std::map<std::string, sessptr> sesss;
        typedef std::map<int, std::pair<sessptr, aug::smartfd> > listeners;
        typedef std::map<augas_id, int> idtofd;
        typedef std::map<int, connptr> conns;

        modules modules_;
        sesss sesss_;
        listeners listeners_;
        conns conns_;
        idtofd idtofd_;

    public:
        void
        clear();

        void
        erase(const connptr& conn);

        void
        insert(const connptr& conn);

        void
        insert(const sessptr& sess, const aug::smartfd& sfd);

        void
        load(const options& options, const augas_host& host);

        bool
        sendall(aug::mplexer& mplexer, augas_id cid, const char* sname,
                const char* buf, size_t size);

        bool
        sendself(aug::mplexer& mplexer, augas_id cid, const char* buf,
                 size_t size);

        void
        sendother(aug::mplexer& mplexer, augas_id cid, const char* sname,
                  const char* buf, size_t size);

        void
        reconf() const;

        void
        teardown() const;

        connptr
        getbyfd(aug::fdref fd) const;

        connptr
        getbyid(augas_id id) const;

        sessptr
        getsess(const std::string& name) const;

        bool
        isconnected() const;

        sessptr
        islistener(aug::fdref fd) const;
    };
}

#endif // AUGAS_MANAGER_HPP
