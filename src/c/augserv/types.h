/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGSERV_TYPES_H
#define AUGSERV_TYPES_H

#include "augext/task.h"

enum aug_option {

    /**
     * The following options are required constants.
     */

    AUG_OPTLONGNAME = 1,
    AUG_OPTSHORTNAME,
    AUG_OPTPROGRAM,
    AUG_OPTEMAIL,

    /**
     * Once aug_readservconf() has been called, the following options may also
     * be requested.  Once requested, they should remain constant thereafter.
     */

    /**
     * Pid file.
     */

    AUG_OPTPIDFILE
};

struct aug_serv {
    const char* (*getopt_)(int);
    aug_result (*readconf_)(const char*, aug_bool, aug_bool);
    aug_task* (*create_)(void);
};

#endif /* AUGSERV_TYPES_H */
