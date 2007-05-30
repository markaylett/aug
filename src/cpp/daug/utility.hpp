/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_UTILITY_HPP
#define DAUG_UTILITY_HPP

struct augrt_module;
struct augrt_object;

namespace augrt {

    void
    setdefaults(augrt_module& dst, const augrt_module& src,
                void (*teardown)(const augrt_object*));
}

#endif // DAUG_UTILITY_HPP
