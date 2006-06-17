/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_STRING_HPP
#define AUGSYSPP_STRING_HPP

#include "augsys/string.h"

#include <algorithm>
#include <functional>
#include <string>

namespace aug {

	struct nocase_less : public std::binary_function<char, char, bool> {
		bool
        operator ()(char lhs, char rhs) const
		{
			return tolower(lhs) < tolower(rhs);
		}
	};

    inline bool
    nocase_compare(const std::string& lhs, const std::string& rhs)
    {
        return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                            rhs.begin(), rhs.end(),
                                            nocase_less());
    }
}

#endif // AUGSYSPP_STRING_HPP
