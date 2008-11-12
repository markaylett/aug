/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGMARPP_COMPAT_HPP
#define AUGMARPP_COMPAT_HPP

#if defined(__GNUC__) && __GNUC__ < 3

#include <cstring>
#include <iostream>

namespace std {

    typedef ios ios_base;

    template <typename charT>
    struct char_traits;

    template <>
    struct char_traits<char> {

        typedef char char_type;
        typedef int int_type;
        typedef streampos pos_type;
        typedef streamoff off_type;
        // typedef mbstate_t state_type;

        static void
        assign(char_type& lhs, const char_type& rhs)
        {
            lhs = rhs;
        }
        static bool
        eq(const char_type& lhs, const char_type& rhs)
        {
            return lhs == rhs;
        }
        static bool
        lt(const char_type& lhs, const char_type& rhs)
        {
            return lhs < rhs;
        }
        static int
        compare(const char_type* lhs, const char_type* rhs, size_t len)
        {
            return memcmp(lhs, rhs, len);
        }
        static size_t
        length(const char_type* src)
        {
            return strlen(src);
        }
        static char_type*
        copy(char_type* to, const char_type* from, size_t len)
        {
            return reinterpret_cast<char_type*>(memcpy(to, from, len));
        }
        static const char_type*
        find(const char_type* src, size_t len, const char_type& ch)
        {
            return reinterpret_cast<const char_type*>(memchr(src, ch, len));
        }
        static char_type*
        move(char_type *to, const char_type *from, size_t len)
        {
            return reinterpret_cast<char_type*>(memmove(to, from, len));
        }
        static char_type*
        assign(char_type *to, size_t len, const char_type& ch)
        {
            return reinterpret_cast<char_type*>(memset(to, ch, len));
        }
        static char_type
        to_char_type(const int_type& ch)
        {
            return static_cast<char_type>(ch);
        }
        static int_type
        to_int_type(const char_type& ch)
        {
            return static_cast<int_type>(static_cast<unsigned char>(ch));
        }
        static bool
        eq_int_type(const int_type& lhs, const int_type& rhs)
        {
            return lhs == rhs;
        }
        static int_type
        eof()
        {
            return EOF;
        }
        static int_type
        not_eof(const int_type& ch)
        {
            return ch != eof() ? ch : !eof();
        }
    };

    template <typename charT, typename char_traitsT = char_traits<charT> >
    class basic_streambuf;

    template <>
    class basic_streambuf<char, char_traits<char> >
        : public streambuf {
    public:
        typedef char char_type;
        typedef char_traits<char_type> char_traits;
        typedef char_traits::int_type int_type;
        typedef char_traits::pos_type pos_type;
        typedef char_traits::off_type off_type;
    };

    template <typename charT, typename char_traitsT = char_traits<charT> >
    class basic_istream;

    template <>
    class basic_istream<char, char_traits<char> >
        : public istream {
    public:
        typedef char char_type;
        typedef char_traits<char_type> char_traits;
        typedef char_traits::int_type int_type;
        typedef char_traits::pos_type pos_type;
        typedef char_traits::off_type off_type;

        explicit
        basic_istream(basic_streambuf<char, char_traits>* buf)
            : istream(buf)
        {
        }
    };

    template <typename charT, typename char_traitsT = char_traits<charT> >
    class basic_ostream;

    template <>
    class basic_ostream<char, char_traits<char> >
        : public ostream {
    public:
        typedef char char_type;
        typedef char_traits<char_type> char_traits;
        typedef char_traits::int_type int_type;
        typedef char_traits::pos_type pos_type;
        typedef char_traits::off_type off_type;

        explicit
        basic_ostream(basic_streambuf<char, char_traits>* buf)
            : ostream(buf)
        {
        }
    };

    template <typename charT, typename char_traitsT = char_traits<charT> >
    class basic_iostream;

    template <>
    class basic_iostream<char, char_traits<char> >
        : public iostream {
    public:
        typedef char char_type;
        typedef char_traits<char_type> char_traits;
        typedef char_traits::int_type int_type;
        typedef char_traits::pos_type pos_type;
        typedef char_traits::off_type off_type;

        explicit
        basic_iostream(basic_streambuf<char, char_traits>* buf)
            : iostream(buf)
        {
        }
    };
}

#endif // __GNUC__ < 3

#endif // AUGMARPP_COMPAT_HPP
