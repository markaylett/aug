/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYSPP_BUILD
#include "augsyspp/base.hpp"

#include "augsys/string.h" // aug_perror()

using namespace aug;
using namespace std;

AUGSYSPP_API
initialiser::~initialiser() NOTHROW
{
    if (-1 == aug_term())
        aug_perror("aug_term() failed");
}

AUGSYSPP_API
initialiser::initialiser()
{
    init();
}
