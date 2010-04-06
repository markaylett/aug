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
#ifndef AUGSERV_OPTIONS_H
#define AUGSERV_OPTIONS_H

#include "augserv/config.h"

#include "augsys/limits.h" /* AUG_PATH_MAX */

#include "augtypes.h"

enum aug_command {

    AUG_CMDDEFAULT,
    AUG_CMDEXIT,
    AUG_CMDINSTALL,
    AUG_CMDRECONF,
    AUG_CMDSTART,
    AUG_CMDSTATUS,
    AUG_CMDSTOP,
    AUG_CMDUNINSTALL
};

struct aug_options {
    char conffile_[AUG_PATH_MAX + 1];
    aug_bool batch_;
    enum aug_command command_;
};

AUGSERV_API aug_result
aug_readopts_N(struct aug_options* options, int argc, char* argv[]);

#define AUG_CONFFILE(x) (*(x)->conffile_ ? (x)->conffile_ : NULL)

#endif /* AUGSERV_OPTIONS_H */
