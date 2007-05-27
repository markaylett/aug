/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file getopt.h
   TODO
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
