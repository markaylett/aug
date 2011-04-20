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
#ifndef AUGMARPP_STREAM_HPP
#define AUGMARPP_STREAM_HPP

#include "augmarpp/streambuf.hpp"
#include "augmarpp/types.hpp"

namespace aug {

    const struct memory_ { } memory = memory_();

    template <typename charT,
              typename char_traitsT = std::char_traits<charT> >
    class basic_imarstream : public std::basic_istream<charT, char_traitsT>,
                             public mpool_ops {
    public:
        typedef charT char_type;
        typedef char_traitsT char_traits_type;
        typedef basic_marstreambuf<char_type,
                                   char_traits_type> streambuf_type;

    private:
        typedef std::basic_istream<char_type, char_traits_type> base_type;
        streambuf_type streambuf_;

    public:
        explicit
        basic_imarstream(marref ref, std::streamsize size = AUG_MAXLINE)
            : base_type(0),
              streambuf_(ref, std::ios_base::in, size)
        {
            rdbuf(&streambuf_);
            if (!streambuf_.is_open())
                base_type::setstate(std::ios_base::failbit);
        }
        explicit
        basic_imarstream(mpoolref mpool, const char* path,
                         int flags = AUG_RDONLY, mode_t mode = 0444,
                         std::streamsize size = AUG_MAXLINE)
            : base_type(0),
              streambuf_(aug_openmar_BIN(mpool.get(), path, flags, mode),
                         std::ios_base::in, size)
        {
            rdbuf(&streambuf_);
            if (!streambuf_.is_open())
                base_type::setstate(std::ios_base::failbit);
        }
        int
        close()
        {
            return streambuf_.close();
        }
        bool
        is_open() const
        {
            return streambuf_.is_open();
        }
    };

    template <typename charT,
              typename char_traitsT = std::char_traits<charT> >
    class basic_omarstream : public std::basic_ostream<charT, char_traitsT>,
                             public mpool_ops {
    public:
        typedef charT char_type;
        typedef char_traitsT char_traits_type;
        typedef basic_marstreambuf<char_type,
                                   char_traits_type> streambuf_type;

    private:
        typedef std::basic_ostream<char_type, char_traits_type> base_type;
        streambuf_type streambuf_;

    public:
        explicit
        basic_omarstream(marref ref, std::streamsize size = AUG_MAXLINE)
            : base_type(0),
              streambuf_(ref, std::ios_base::out, size)
        {
            rdbuf(&streambuf_);
            if (!streambuf_.is_open())
                base_type::setstate(std::ios_base::failbit);
        }
        explicit
        basic_omarstream(mpoolref mpool, const char* path,
                         int flags = AUG_WRONLY | AUG_CREAT,
                         mode_t mode = 0664,
                         std::streamsize size = AUG_MAXLINE)
            : base_type(0),
              streambuf_(aug_openmar_BIN(mpool.get(), path, flags, mode),
                         std::ios_base::out, size)
        {
            rdbuf(&streambuf_);
            if (!streambuf_.is_open())
                base_type::setstate(std::ios_base::failbit);
        }
        int
        close()
        {
            return streambuf_.close();
        }
        bool
        is_open() const
        {
            return streambuf_.is_open();
        }
    };

    template <typename charT,
              typename char_traitsT = std::char_traits<charT> >
    class basic_iomarstream
        : public std::basic_iostream<charT, char_traitsT>,
          public mpool_ops {
    public:
        typedef charT char_type;
        typedef char_traitsT char_traits_type;
        typedef basic_marstreambuf<char_type,
                                   char_traits_type> streambuf_type;

    private:
        typedef std::basic_iostream<char_type, char_traits_type> base_type;
        streambuf_type streambuf_;

    public:
        basic_iomarstream(mpoolref mpool, const memory_&,
                          std::streamsize size = AUG_MAXLINE)
            : base_type(0),
              streambuf_(aug_createmar_BIN(mpool.get()), std::ios_base::in
                         | std::ios_base::out, size)
        {
            rdbuf(&streambuf_);
            if (!streambuf_.is_open())
                base_type::setstate(std::ios_base::failbit);
        }
        basic_iomarstream(mpoolref mpool, const char* path,
                          int flags = AUG_RDWR | AUG_CREAT,
                          mode_t mode = 0664,
                          std::streamsize size = AUG_MAXLINE)
            : base_type(0),
              streambuf_(aug_openmar_BIN(mpool.get(), path, flags, mode),
                         std::ios_base::in | std::ios_base::out, size)
        {
            rdbuf(&streambuf_);
            if (!streambuf_.is_open())
                base_type::setstate(std::ios_base::failbit);
        }
        explicit
        basic_iomarstream(marref ref, std::streamsize size = AUG_MAXLINE)
            : base_type(0),
              streambuf_(ref, std::ios_base::in | std::ios_base::out, size)
        {
            rdbuf(&streambuf_);
            if (!streambuf_.is_open())
                base_type::setstate(std::ios_base::failbit);
        }
        int
        close()
        {
            return streambuf_.close();
        }
        bool
        is_open() const
        {
            return streambuf_.is_open();
        }
    };

    typedef basic_imarstream<char> imarstream;
    typedef basic_omarstream<char> omarstream;
    typedef basic_iomarstream<char> iomarstream;
}

#endif // AUGMARPP_STREAM_HPP
