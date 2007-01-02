/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_STRING_HPP
#define AUGSYSPP_STRING_HPP

#include "augsys/string.h"

#include <cctype>
#include <functional>

namespace aug {

	struct nocase_less : public std::binary_function<char, char, bool> {
		bool
        operator ()(char lhs, char rhs) const
		{
			return tolower(lhs) < tolower(rhs);
		}
	};
}

#endif // AUGSYSPP_STRING_HPP
