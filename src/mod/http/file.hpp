/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGAS_FILE_HPP
#define AUGAS_FILE_HPP

#include "augext/blob.h"

#include <stdexcept>
#include <vector>

namespace aug {

    class http_error : public std::domain_error {
        const int code_;
    public:
        http_error(int code, const std::string& title)
            : std::domain_error(title),
              code_(code)
        {
        }
        int
        code() const
        {
            return code_;
        }
    };

    blobptr
    getfile(const char* path);

    std::string
    jointype(const std::vector<std::string>& nodes);

    std::string
    joinpath(const char* root, const std::vector<std::string>& nodes);

    void
    loadmimetypes();

    std::string
    mimetype(const std::string& path);

    std::vector<std::string>
    splitpath(const std::string& path);
}

#endif // AUGAS_FILE_HPP
