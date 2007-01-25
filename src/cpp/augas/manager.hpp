/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
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
        typedef std::map<augas_id, int, std::greater<augas_id> > idtofd;
        typedef std::map<int, fileptr> files;

        modules modules_;
        sesss sesss_;
        files files_;
        idtofd idtofd_;

        manager(const manager& rhs);

        manager&
        operator =(const manager& rhs);

        void
        insert(const std::string& name, const sessptr& sess);

    public:
        manager()
        {
        }
        void
        clear();

        void
        erase(const file_base& file);

        void
        insert(const fileptr& file);

        void
        update(const fileptr& file, aug::fdref prev);

        void
        load(const char* rundir, const options& options,
             const augas_host& host);

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

    class scoped_insert {

        manager& manager_;
        fileptr file_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != file_)
                manager_.erase(*file_);
        }
        scoped_insert(manager& manager, const fileptr& file)
            : manager_(manager),
              file_(file)
        {
            manager.insert(file);
        }
        void
        commit()
        {
            file_ = fileptr();
        }
    };
}

#endif // AUGAS_MANAGER_HPP
