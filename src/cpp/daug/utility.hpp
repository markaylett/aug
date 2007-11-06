/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_UTILITY_HPP
#define DAUG_UTILITY_HPP

struct augmod_control;
struct augmod_object;

namespace augrt {

    void
    setdefaults(augmod_control& dst, const augmod_control& src,
                void (*teardown)(const augmod_object*));
}

#endif // DAUG_UTILITY_HPP
