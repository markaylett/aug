/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_UTILITY_HPP
#define DAUG_UTILITY_HPP

struct augmod_proxy;
struct augmod_object;

namespace augas {

    void
    setdefaults(augmod_proxy& dst, const augmod_proxy& src,
                void (*teardown)(const augmod_object*));
}

#endif // DAUG_UTILITY_HPP
