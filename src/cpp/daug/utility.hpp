/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_UTILITY_HPP
#define DAUG_UTILITY_HPP

struct mod_module;
struct mod_handle;

namespace daug {

    void
    setdefaults(mod_module& dst, const mod_module& src,
                void (*teardown)(const mod_handle*));
}

#endif // DAUG_UTILITY_HPP
