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
#define AUGARCPP_BUILD
#include "augarcpp/base.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augarcpp/names.hpp"

using namespace aug;
using namespace std;

#include "augarcpp/recur.hpp"
#include "augarcpp/utility.hpp"

namespace {

    class inner : public inner_base {
        const factory_base& factory_;
        names names_;
        typedptr special_;
    protected:

        // node_base

        void
        do_clear()
        {
            names_.clear();
            if (null != special_)
                special_.reset();
        }

        unsigned
        do_depth() const
        {
            return std::max(names_.depth(), null == special_ ? 0
                            : 1 + special_->depth());
        }

        bool
        do_ismatch(path_iterator pit, path_iterator pend) const
        {
            const match m(do_query(pit, pend));
            return ARCTRANSIT == m || ARCINCLUDE == m;
        }

        void
        do_print(const string& path, ostream& os) const
        {
            names_.print(path, os);
            if (null != special_)
                special_->print(path, os);
        }

        // query_base

        match
        do_query(path_iterator pit, path_iterator pend) const
        {
            if (pit == pend)
                return ARCNOTHING;
            const match m(names_.query(pit, pend));
            return null == special_ ? m
                : foldmatch(m, special_->query(pit, pend));
        }

        bool
        do_isdead() const
        {
            return names_.isdead()
                && (null == special_ || special_->isdead());
        }

        // inner_base

        void
        do_insertname(const string& name)
        {
            names_.insertname(name);
            if (null != special_)
                special_->insertname(name);
        }

        void
        do_insertname(const string& name, path_iterator pit,
                       path_iterator pend)
        {
            names_.insertname(name, pit, pend);
            if (null != special_)
                special_->insertname(name, pit, pend);
        }

        void
        do_insertrecur()
        {
            names_.insertrecur();
            // Promotion from wildcard to recursive.
            if (null == special_ || ARCRECUR != special_->type())
                special_.reset(new recur(factory_));
            special_->insertrecur();
        }

        void
        do_insertwild()
        {
            names_.insertwild();
            // Promotion from wildcard to recursive.
            if (null == special_)
                special_.reset(new wild(factory_));
            special_->insertwild();
        }

        void
        do_insertwild(path_iterator pit, path_iterator pend)
        {
            names_.insertwild(pit, pend);
            // Promotion from wildcard to recursive.
            if (null == special_)
                special_.reset(new wild(factory_));
            special_->insertwild(pit, pend);
        }

        void
        do_erasename(const string& name)
        {
            names_.erasename(name);
            if (null != special_) {
                special_->erasename(name);
                if (special_->isdead())
                    special_.reset();
            }
        }

        void
        do_erasename(const string& name, path_iterator pit,
                      path_iterator pend)
        {
            names_.erasename(name, pit, pend);
            if (null != special_) {
                special_->erasename(name, pit, pend);
                if (special_->isdead())
                    special_.reset();
            }
        }

        void
        do_eraserecur()
        {
            names_.eraserecur();
            special_.reset();
        }

        void
        do_erasewild()
        {
            names_.erasewild();
            if (null != special_) {
                special_->erasewild();
                if (special_->isdead())
                    special_.reset();
            }
        }

        void
        do_erasewild(path_iterator pit, path_iterator pend)
        {
            names_.erasewild(pit, pend);
            if (null != special_) {
                special_->erasewild(pit, pend);
                if (special_->isdead())
                    special_.reset();
            }
        }

    public:

        ~inner() AUG_NOTHROW
        {
        }

        explicit
        inner(const factory_base& factory)
            : factory_(factory),
              names_(factory)
        {
        }
    };

    class outer : public inner, public outer_base {

        match match_;

    protected:

        // node_base

        void
        do_clear()
        {
            inner::do_clear();
            match_ = ARCNOTHING;
        }

        unsigned
        do_depth() const
        {
            return inner::do_depth();
        }

        bool
        do_ismatch(path_iterator pit, path_iterator pend) const
        {
            const match m(do_query(pit, pend));
            return ARCTRANSIT == m || ARCINCLUDE == m;
        }

        void
        do_print(const string& path, ostream& os) const
        {
            if (ARCINCLUDE == match_)
                os << path << '\n';
            else if (ARCEXCLUDE == match_)
                os << '!' << path << '\n';
            return inner::do_print(path, os);
        }

        // query_base

        match
        do_query(path_iterator pit, path_iterator pend) const
        {
            return pit == pend ? match_
                : inner::do_query(pit, pend);
        }

        bool
        do_isdead() const
        {
            return ARCINCLUDE != match_ && inner::do_isdead();
        }

        // inner_base

        void
        do_insertname(const string& name)
        {
            inner::do_insertname(name);
            if (ARCINCLUDE != match_)
                match_ = ARCTRANSIT;
        }

        void
        do_insertname(const string& name, path_iterator pit,
                       path_iterator pend)
        {
            inner::do_insertname(name, pit, pend);
            if (ARCINCLUDE != match_)
                match_ = ARCTRANSIT;
        }

        void
        do_insertrecur()
        {
            inner::do_insertrecur();
            if (ARCINCLUDE != match_)
                match_ = ARCTRANSIT;
        }

        void
        do_insertwild()
        {
            inner::do_insertwild();
            if (ARCINCLUDE != match_)
                match_ = ARCTRANSIT;
        }

        void
        do_insertwild(path_iterator pit, path_iterator pend)
        {
            inner::do_insertwild(pit, pend);
            if (ARCINCLUDE != match_)
                match_ = ARCTRANSIT;
        }

        void
        do_erasename(const string& name)
        {
            inner::do_erasename(name);
        }

        void
        do_erasename(const string& name, path_iterator pit,
                      path_iterator pend)
        {
            inner::do_erasename(name, pit, pend);
        }

        void
        do_eraserecur()
        {
            inner::do_eraserecur();
        }

        void
        do_erasewild()
        {
            inner::do_erasewild();
        }

        void
        do_erasewild(path_iterator pit, path_iterator pend)
        {
            inner::do_erasewild(pit, pend);
        }

        // outer_base

        void
        do_insert(const string& head, path_iterator pit,
                  path_iterator pend)
        {
            insertnode(head, pit, pend, *this);
        }

        void
        do_insert(const string& path)
        {
            insertnode(path, *this);
        }

        void
        do_erase(const string& head, path_iterator pit, path_iterator pend)
        {
            erasenode(head, pit, pend, *this);
        }

        void
        do_erase(const string& path)
        {
            erasenode(path, *this);
        }

        void
        do_setmatch(match m)
        {
            match_ = m;
        }

    public:

        ~outer() AUG_NOTHROW
        {
        }

        explicit
        outer(const factory_base& factory)
            : inner(factory),
              match_(ARCNOTHING)
        {
        }

    };
}

AUGARCPP_API innerptr
factory::do_createinner() const
{
    return innerptr(new inner(*this));
}

AUGARCPP_API outerptr
factory::do_createouter() const
{
    return outerptr(new outer(*this));
}

AUGARCPP_API
factory::~factory() AUG_NOTHROW
{
}
