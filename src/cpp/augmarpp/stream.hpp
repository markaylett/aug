/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
              streambuf_(aug_openmar(mpool.get(), path, flags, mode),
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
              streambuf_(aug_openmar(mpool.get(), path, flags, mode),
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
              streambuf_(aug_createmar(mpool.get()), std::ios_base::in
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
              streambuf_(aug_openmar(mpool.get(), path, flags, mode),
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
