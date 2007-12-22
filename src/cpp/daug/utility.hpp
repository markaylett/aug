/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_UTILITY_HPP
#define DAUG_UTILITY_HPP

struct aum_module;
struct aum_handle;

namespace daug {

    void
    setdefaults(aum_module& dst, const aum_module& src,
                void (*teardown)(const aum_handle*));
}

#endif // DAUG_UTILITY_HPP
