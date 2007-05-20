/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_MANAGER_HPP
#define DAUG_MANAGER_HPP

#include "augrtpp/object.hpp"
#include "augrtpp/serv.hpp"

#include <map>
#include <vector>

namespace augas {

    class options;

    class manager {

        typedef std::map<std::string, aug::servptr> servs;
        typedef std::multimap<std::string, aug::servptr> groups;
        typedef std::map<int, aug::objectptr> socks;
        typedef std::map<augas_id, int, std::greater<augas_id> > idtofd;

        servs servs_;
        groups groups_, temp_;
        socks socks_;
        idtofd idtofd_;

        manager(const manager& rhs);

        manager&
        operator =(const manager& rhs);

    public:
        manager()
        {
        }
        bool
        append(augas_id cid, const aug_var& var);

        bool
        append(augas_id cid, const void* buf, size_t size);

        void
        insert(const std::string& name, const aug::servptr& serv,
               const char* groups);

        void
        clear();

        void
        erase(const aug::object_base& sock);

        void
        insert(const aug::objectptr& sock);

        void
        update(const aug::objectptr& sock, aug::fdref prev);

        void
        load(const char* rundir, const options& options,
             const augas_host& host, void (*teardown)(const augas_object*));

        void
        teardown();

        void
        reconf() const;

        aug::objectptr
        getbyfd(aug::fdref fd) const;

        aug::objectptr
        getbyid(augas_id id) const;

        aug::servptr
        getserv(const std::string& name) const;

        void
        getservs(std::vector<aug::servptr>& servs,
                 const std::string& alias) const;

        bool
        empty() const;
    };

    class scoped_insert {

        manager& manager_;
        aug::objectptr sock_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != sock_)
                manager_.erase(*sock_);
        }
        scoped_insert(manager& manager, const aug::objectptr& sock)
            : manager_(manager),
              sock_(sock)
        {
            manager.insert(sock);
        }
        void
        commit()
        {
            sock_ = aug::objectptr();
        }
    };
}

#endif // DAUG_MANAGER_HPP
