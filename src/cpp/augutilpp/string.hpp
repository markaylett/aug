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
#ifndef AUGUTILPP_STRING_HPP
#define AUGUTILPP_STRING_HPP

#include "augctx/string.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <typeinfo> // std::bad_cast
#include <vector>

namespace aug {

    namespace detail {
        const char SPACE[] = " \t\n\v\f\r";
        static const char HEX[] = "0123456789ABCDEF";
    }

	struct nocase_less : public std::binary_function<char, char, bool> {
		bool
        operator ()(char lhs, char rhs) const
		{
			return tolower(lhs) < tolower(rhs);
		}
	};

    inline char
    lcase(char ch)
    {
        return std::tolower(ch); // Second argument is locale.
    }

    inline char
    ucase(char ch)
    {
        return std::toupper(ch); // Second argument is locale.
    }

    inline std::string&
    ltrim(std::string& s, const char* delims = detail::SPACE)
    {
        s.erase(0, s.find_first_not_of(delims));
        return s;
    }

    inline std::string
    ltrimcopy(const std::string& s, const char* delims = detail::SPACE)
    {
        std::string::size_type pos(s.find_first_not_of(delims));
        return std::string::npos == pos
            ? std::string() : s.substr(pos);
    }

    inline std::string&
    rtrim(std::string& s, const char* delims = detail::SPACE)
    {
        std::string::size_type pos(s.find_last_not_of(delims));
        if (std::string::npos == pos)
            s.clear();
        else
            s.erase(pos + 1);
        return s;
    }

    inline std::string
    rtrimcopy(const std::string& s, const char* delims = detail::SPACE)
    {
        std::string::size_type pos(s.find_last_not_of(delims));
        return std::string::npos == pos
            ? std::string() : s.substr(0, pos + 1);
    }

    inline std::string&
    trim(std::string& s, const char* delims = detail::SPACE)
    {
        return rtrim(ltrim(s, delims), delims);
    }

    inline std::string
    trimcopy(const std::string& s, const char* delims = detail::SPACE)
    {
        return rtrimcopy(ltrimcopy(s, delims));
    }

    template <typename T, typename U, typename V>
    T
    copyif(T it, T end, U dst, V pred)
    {
        for (; it != end && pred(*it); ++it, ++dst)
            *dst = *it;
        return it;
    }

    template <typename T, typename U, typename V>
    T
    copybackif(T it, T end, U& dst, V pred)
    {
        return copyif(it, end, std::back_inserter(dst), pred);
    }

    template <typename T, typename U, typename V>
    T
    copyneq(T it, T end, U dst, V delim)
    {
        return copyif(it, end, dst,
                      std::bind2nd(std::not_equal_to<V>(), delim));
    }

    template <typename T, typename U, typename V>
    T
    copybackneq(T it, T end, U& dst, V delim)
    {
        return copyif(it, end, std::back_inserter(dst),
                      std::bind2nd(std::not_equal_to<V>(), delim));
    }

    template <typename T, typename U, typename V, typename W>
    W
    tokenise(T it, T end, U& tok, V delim, W fn)
    {
        for (; (it = copybackneq(it, end, tok, delim)) != end; ++it) {
            fn(tok);
            tok.clear();
        }
        return fn;
    }

    namespace detail {
        template <typename T, typename U>
        class tokens {
            T it_;
        public:
            explicit
            tokens(T it)
                : it_(it)
            {
            }
            void
            operator ()(U& x)
            {
                *it_++ = x;
            }
        };
        template <typename T, typename U>
        tokens<U, T>
        maketokens(U it)
        {
            return tokens<U, T>(it);
        }
    }

    template <typename T, typename U, typename V>
    std::vector<U>
    tokenise(T it, T end, U& tok, V delim)
    {
        std::vector<U> v;
        tokenise(it, end, tok, delim,
                 detail::maketokens<U>(std::back_inserter(v)));
        return v;
    }

    template <typename T, typename U, typename V>
    V
    splitn(T it, T end, U delim, V fn)
    {
        std::string tok;
        fn = tokenise(it, end, tok, delim, fn);
        fn(tok);
        return fn;
    }

    template <typename T>
    std::vector<std::string>
    splitn(T it, T end, char delim)
    {
        std::vector<std::string> v;
        splitn(it, end, delim,
               detail::maketokens<std::string>(std::back_inserter(v)));
        return v;
    }

    template <typename T, typename U, typename V>
    bool
    split2(T it, T end, U& first, U& second, V delim)
    {
        if ((it = copybackneq(it, end, first, delim)) == end)
            return false;

        std::copy(++it, end, std::back_inserter(second));
        return true;
    }

    inline int
    xdigitoi(char ch)
    {
        return '0' <= ch && ch <= '9'
            ? ch - '0' : std::toupper(ch) - 'A' + 10;
    }

    template <typename T, typename U>
    U
    urlencode(T it, T end, U dst)
    {
        for (; it != end; ++it, ++dst)

            if (std::isalnum(*it))
                *dst = *it;
            else
                switch (*it) {
                case ' ':
                    *dst = '+';
                    break;
                case '-':
                case '_':
                case '.':
                case '!':
                case '~':
                case '*':
                case '\'':
                case '(':
                case ')':
                    *dst = *it;
                    break;
                default:
                    *dst++ = '%';
                    *dst++ = detail::HEX[*it / 16];
                    *dst = detail::HEX[*it % 16];
                }

        return dst;
    }

    template <typename T>
    std::string
    urlencode(T it, T end)
    {
        std::string s;
        urlencode(it, end, std::back_inserter(s));
        return s;
    }

    template <typename T, typename U>
    U
    urlpack(T it, T end, U dst)
    {
        for (bool first(true); it != end; ++it) {
            if (first)
                first = false;
            else
                *dst++ = '&';
            urlencode(it->first.begin(), it->first.end(), dst);
            *dst++ = '=';
            urlencode(it->second.begin(), it->second.end(), dst);
        }
        return dst;
    }

    template <typename T>
    std::string
    urlpack(T it, T end)
    {
        std::string s;
        urlpack(it, end, std::back_inserter(s));
        return s;
    }

    template <typename T, typename U>
    U
    urldecode(T it, T end, U dst)
    {
        for (; it != end; ++it, ++dst)
            switch (*it) {
            case '+':
                *dst = ' ';
                break;
            case '%':
                if (2 < distance(it, end) && std::isxdigit(it[1])
                    && std::isxdigit(it[2])) {
                    *dst = static_cast<char>(xdigitoi(it[1]) * 16
                                             + xdigitoi(it[2]));
                    it += 2;
                    break;
                }
            default:
                *dst = *it;
                break;
            }

        return dst;
    }

    template <typename T>
    std::string
    urldecode(T it, T end)
    {
        std::string s;
        urldecode(it, end, std::back_inserter(s));
        return s;
    }

    namespace detail {
        template <typename T>
        struct urlpairs {
            T it_;
            explicit
            urlpairs(T it)
                : it_(it)
            {
            }
            void
            operator ()(std::string& s)
            {
                std::pair<std::string, std::string> xy;
                split2(s.begin(), s.end(), xy.first, xy.second, '=');
                xy.first = urldecode(xy.first.begin(), xy.first.end());
                xy.second = urldecode(xy.second.begin(), xy.second.end());
                *it_++ = xy;
            }
        };
        template <typename T>
        urlpairs<T>
        makeurlpairs(T it)
        {
            return urlpairs<T>(it);
        }
    }

    template <typename T, typename U>
    U
    urlunpack(T it, T end, U dst)
    {
        return splitn(it, end, '&', detail::makeurlpairs(dst)).it_;
    }

    template <typename T>
    std::vector<std::pair<std::string, std::string> >
    urlunpack(T it, T end)
    {
        std::vector<std::pair<std::string, std::string> > v;
        splitn(it, end, '&',
               detail::makeurlpairs(std::back_inserter(v)));
        return v;
    }

    template<typename T>
    bool
    getvalue(const std::map<std::string, std::string>& pairs,
             const std::string& key, T& value)
    {
        std::map<std::string, std::string>
            ::const_iterator it(pairs.find(key));
        if (it == pairs.end())
            return false;

        std::istringstream ss(it->second);
        if (!(ss >> value))
            throw std::bad_cast();

        return true;
    }

    template<typename T>
    T
    getvalue(const std::map<std::string, std::string>& pairs,
             const std::string& key)
    {
        T value = T();
        getvalue(pairs, key, value);
        return value;
    }
}

#endif // AUGUTILPP_STRING_HPP
