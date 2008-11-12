/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
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
