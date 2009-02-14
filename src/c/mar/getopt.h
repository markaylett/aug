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
#ifndef MAR_GETOPT_H
#define MAR_GETOPT_H

#include "mar/config.h"

AUG_EXTERNC char* ntp_optarg;
AUG_EXTERNC int ntp_optind;
AUG_EXTERNC int ntp_optopt;
AUG_EXTERNC int ntp_opterr;

AUG_EXTERNC int
ntp_getopt(int argc, char** argv, const char* optstring);

#define aug_optarg ntp_optarg
#define aug_optind ntp_optind
#define aug_optopt ntp_optopt
#define aug_opterr ntp_opterr
#define aug_getopt ntp_getopt

#endif /* MAR_GETOPT_H */
