/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_STRING_HPP
#define AUGSYSPP_STRING_HPP

#include "augctx/string.h"

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
