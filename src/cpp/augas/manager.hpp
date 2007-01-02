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
        typedef std::map<augas_id, int> idtofd;
        typedef std::map<int, fileptr> files;

        modules modules_;
        sesss sesss_;
        files files_;
        idtofd idtofd_;

        void
        insert(const std::string& name, const sessptr& sess);

    public:
        void
        clear();

        void
        erase(const file_base& file);

        void
        insert(const fileptr& file);

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
        teardown();

        void
        reconf() const;

        fileptr
        getbyfd(aug::fdref fd) const;

        fileptr
        getbyid(augas_id id) const;

        sessptr
        getsess(const std::string& name) const;

        bool
        empty() const;
    };
}

#endif // AUGAS_MANAGER_HPP
