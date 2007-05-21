/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_OBJECTS_HPP
#define AUGRTPP_OBJECTS_HPP

#include "augrtpp/object.hpp"
#include "augrtpp/serv.hpp"

#include <map>

namespace aug {

    class objects {

        typedef std::map<int, objectptr> socks;
        typedef std::map<augas_id, int, std::greater<augas_id> > idtofd;

        socks socks_;
        idtofd idtofd_;

        objects(const objects& rhs);

        objects&
        operator =(const objects& rhs);

    public:
        objects()
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
        update(const objectptr& sock, fdref prev);

        void
        teardown();

        objectptr
        getbyfd(fdref fd) const;

        objectptr
        getbyid(augas_id id) const;

        bool
        empty() const;
    };

    class scoped_insert {

        objects& objects_;
        objectptr sock_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != sock_)
                objects_.erase(*sock_);
        }
        scoped_insert(objects& objects, const objectptr& sock)
            : objects_(objects),
              sock_(sock)
        {
            objects.insert(sock);
        }
        void
        commit()
        {
            sock_ = objectptr();
        }
    };
}

#endif // AUGRTPP_OBJECTS_HPP
