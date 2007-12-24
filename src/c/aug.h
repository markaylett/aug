/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUG_H
#define AUG_H

/**
 * @file aug.h
 *
 * The =aug= package consists of the following libraries:
 * @li augmar.h
 * @li augnet.h
 * @li augob.h
 * @li augsrv.h
 * @li augsys.h
 * @li augutil.h
 *
 * These libraries are written in standard C.  They are targeted at Windows
 * and other POSIX-compliant OS-es.  On Windows, service applications run as
 * NT services; on Linux, they run as daemon processes.
 */

/**
 * @mainpage aug
 *
 * =aug= - a framework for building network servers.
 *
 * This package is partioned into the following namespaces:
 * @li aub.h
 * @li aug.h
 * @li aum.h
 *
 * Tools within aug package are:
 * @li @subpage aubidl
 * @li @subpage daug
 * @li @subpage htdigest
 * @li @subpage mar
 */

#include "augmar.h"
#include "augnet.h"
#include "augob.h"
#include "augsrv.h"
#include "augsys.h"
#include "augutil.h"

#endif // AUG_H
