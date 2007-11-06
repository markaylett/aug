/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_SSL_HPP
#define DAUG_SSL_HPP

#include "augaspp/ssl.hpp"

#include <map>

namespace augas {

    class options;

    typedef std::map<std::string, aug::sslctxptr> sslctxs;

    aug::sslctxptr
    createsslctx(const std::string& name, const options& options,
                 char* frobpass);

    void
    createsslctxs(sslctxs& sslctxs, const options& options,
                  char* frobpass);
}

#endif // DAUG_SSL_HPP
