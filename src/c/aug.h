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
#ifndef AUG_H
#define AUG_H

/**
 * @file aug.h
 *
 * The aug package consists of the following libraries:
 * @li augctx.h
 * @li augext.h
 * @li augmar.h
 * @li augnet.h
 * @li augserv.h
 * @li augsys.h
 * @li augutil.h
 *
 * These libraries are written in standard C.  They are targeted at Windows
 * and other POSIX-compliant OS-es.  The aug package also contains C++
 * wrappers for these libraries.
 */

/**
 * @mainpage aug
 *
 * aug - a framework for building network servers.
 *
 * This package is partioned into the following namespaces:
 * @li aug.h
 * @li augabi.h
 * @li augmod.h
 *
 * Tools within aug package are:
 * @li @subpage augd
 * @li @subpage augidl
 * @li @subpage htdigest
 * @li @subpage mar
 */

#include "augctx.h"
#include "augext.h"
#include "augmar.h"
#include "augnet.h"
#include "augserv.h"
#include "augsys.h"
#include "augutil.h"

#endif /* AUG_H */
