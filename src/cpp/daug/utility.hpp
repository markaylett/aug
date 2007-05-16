/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_UTILITY_HPP
#define DAUG_UTILITY_HPP

struct augas_module;
struct augas_object;

namespace augas {

    void
    setdefaults(augas_module& dst, const augas_module& src,
                void (*teardown)(const augas_object*));
}

#endif // DAUG_UTILITY_HPP
