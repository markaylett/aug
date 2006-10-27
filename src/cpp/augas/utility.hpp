/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_UTILITY_HPP
#define AUGAS_UTILITY_HPP

struct augas_module;

namespace augas {

    void
    setdefaults(augas_module& dst, const augas_module& src);
}

#endif // AUGAS_UTILITY_HPP
