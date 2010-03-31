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
#ifndef AUGSERV_BASE_H
#define AUGSERV_BASE_H

#include "augserv/config.h"

#include "augutil/event.h" /* aug_events_t */

#include "augsys/types.h"

#include "augtypes.h"

struct aug_serv;

#if defined(AUGSERV_BUILD)
AUG_EXTERNC void
aug_setserv_(const struct aug_serv* serv);
#endif /* AUGSERV_BUILD */

AUGSERV_API const char*
aug_getservopt(int opt);

AUGSERV_API aug_result
aug_readservconf(const char* conffile, aug_bool batch, aug_bool daemon);

AUGSERV_API aug_result
aug_initserv_AIN(void);

AUGSERV_API aug_result
aug_runserv(void);

AUGSERV_API void
aug_termserv(void);

AUGSERV_API aug_events_t
aug_events(void);

#endif /* AUGSERV_BASE_H */
