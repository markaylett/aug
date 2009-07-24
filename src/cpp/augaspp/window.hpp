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
#ifndef AUGASPP_WINDOW_HPP
#define AUGASPP_WINDOW_HPP

#include "augaspp/config.hpp"

#include "augnet/packet.h"

#include "augctx/base.h"

#include "augext/clock.h"

#include <exception>
#include <iosfwd>

namespace aug {

    struct AUGASPP_API window_exception : std::exception {
        ~window_exception() AUG_NOTHROW;
    };

    struct AUGASPP_API discard_exception : window_exception {
        ~discard_exception() AUG_NOTHROW;
        const char*
        what() const throw();
    };

    struct AUGASPP_API timeout_exception : window_exception {
        ~timeout_exception() AUG_NOTHROW;
        const char*
        what() const throw();
    };

    typedef unsigned long seqno_t;

    class AUGASPP_API window {
        struct message {
            aug_packet pkt_;
            aug_timeval tv_;
        };
        enum state {
            // First packet pending.
            START,
            // Initial packet ordering.
            PRIME,
            // Fully initialised state.
            READY
        };
        const unsigned size_;
        message* const ring_;
        seqno_t begin_, end_;
        state state_;

        static void
        clear(message& m);

        static bool
        empty(const message& m);

    public:
        ~window() AUG_NOTHROW;

        explicit
        window(unsigned size);

        void
        insert(const aug_packet& pkt, const aug_timeval& tv);

        bool
        next(aug_packet& pkt, aug_timeval& tv);

        void
        drop();

        void
        print(std::ostream& os) const;

        void
        sync();

        bool
        empty() const;

        bool
        ready() const;
    };

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER

    class AUGASPP_API expirywindow {
        clockptr clock_;
        window window_;
        aug_timeval timeout_;
        aug_timeval expiry_;
    public:
        explicit
        expirywindow(clockref clock, unsigned size, unsigned timeout);

        void
        insert(const aug_packet& pkt);

        bool
        next(aug_packet& pkt);

        void
        print(std::ostream& os) const;

        unsigned
        expiry() const;
    };

#if defined(_MSC_VER)
# pragma warning(pop)
#endif // _MSC_VER
}

#endif // AUGASPP_WINDOW_HPP
