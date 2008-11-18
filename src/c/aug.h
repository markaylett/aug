/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
 * @li @subpage augidl
 * @li @subpage daug
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
