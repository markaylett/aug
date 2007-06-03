/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_SSL_HPP
#define DAUG_SSL_HPP

#include "augrtpp/ssl.hpp"

#include <map>

namespace augrt {

    class options;

    typedef std::map<std::string, aug::sslctxptr> sslctxs;

    aug::sslctxptr
    createsslctx(const std::string& name, const options& options,
                 const std::string& pass64);

    void
    createsslctxs(sslctxs& sslctxs, const options& options,
                  const std::string& pass64);
}

#endif // DAUG_SSL_HPP
