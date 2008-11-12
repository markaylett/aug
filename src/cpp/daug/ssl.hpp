/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_SSL_HPP
#define DAUG_SSL_HPP

#include "augaspp/ssl.hpp"

#include <map>

namespace daug {

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
