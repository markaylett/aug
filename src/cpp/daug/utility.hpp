/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_UTILITY_HPP
#define DAUG_UTILITY_HPP

struct maud_module;
struct maud_object;

namespace augas {

    void
    setdefaults(maud_module& dst, const maud_module& src,
                void (*teardown)(const maud_object*));
}

#endif // DAUG_UTILITY_HPP
