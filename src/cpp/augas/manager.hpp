/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MANAGER_HPP
#define AUGAS_MANAGER_HPP

#include "augas/conn.hpp"

#include <map>
#include <vector>

namespace augas {

    class options;

    class manager {

        typedef std::map<std::string, moduleptr> modules;
        typedef std::map<std::string, servptr> servs;
        typedef std::multimap<std::string, servptr> groups;
        typedef std::map<int, objectptr> socks;
        typedef std::map<augas_id, int, std::greater<augas_id> > idtofd;

        modules modules_;
        servs servs_;
        groups groups_, temp_;
        socks socks_;
        idtofd idtofd_;

        manager(const manager& rhs);

        manager&
        operator =(const manager& rhs);

        void
        insert(const std::string& name, const servptr& serv,
               const char* groups);

    public:
        manager()
        {
        }
        bool
        append(augas_id cid, const aug_var& var);

        bool
        append(augas_id cid, const void* buf, size_t size);

        void
        clear();

        void
        erase(const object_base& sock);

        void
        insert(const objectptr& sock);

        void
        update(const objectptr& sock, aug::fdref prev);

        void
        load(const char* rundir, const options& options,
             const augas_host& host);

        void
        teardown();

        void
        reconf() const;

        objectptr
        getbyfd(aug::fdref fd) const;

        objectptr
        getbyid(augas_id id) const;

        servptr
        getserv(const std::string& name) const;

        void
        getservs(std::vector<servptr>& servs, const std::string& alias) const;

        bool
        empty() const;
    };

    class scoped_insert {

        manager& manager_;
        objectptr sock_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != sock_)
                manager_.erase(*sock_);
        }
        scoped_insert(manager& manager, const objectptr& sock)
            : manager_(manager),
              sock_(sock)
        {
            manager.insert(sock);
        }
        void
        commit()
        {
            sock_ = objectptr();
        }
    };
}

#endif // AUGAS_MANAGER_HPP
