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

#include "augutilpp.hpp"
#include "augctxpp.hpp"

using namespace aug;
using namespace std;

int
main(int argc, char* argv[])
{
    try {
        autotlx();

        // isabs()

        aug_check(isabs("/"));
        aug_check(!isabs("."));
        aug_check(!isabs("a"));

#if defined(_WIN32)
        aug_check(isabs("\\"));
        aug_check(isabs("C:/"));
        aug_check(isabs("D:\\"));
#endif // _WIN32

        // abspath()

        aug_check(abspath("/a", "/b") == "/b");
        aug_check(abspath("/a", "b") == "/a/b");

#if defined(_WIN32)
        aug_check(abspath("C:/a", "D:/b") == "D:/b");
        aug_check(abspath("C:\\a", "D:\\b") == "D:\\b");

        aug_check(abspath("C:/a", "b") == "C:/a/b");
        aug_check(abspath("C:\\a", "b") == "C:\\a/b");
#endif // _WIN32

        // joinpath()

        aug_check(joinpath(0, "a") == "a");
        aug_check(joinpath("", "a") == "a");
        aug_check(joinpath("a", "b") == "a/b");
        aug_check(joinpath("a/", "b") == "a/b");
        aug_check(joinpath("a/b", "c/d") == "a/b/c/d");
        return 0;
    } AUG_PERRINFOCATCH;
    return 1;
}
