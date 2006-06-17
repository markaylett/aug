/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file getopt.h
 * \brief TODO
 */

#ifndef MAR_GETOPT_H
#define MAR_GETOPT_H

#include "mar/config.h"

MAR_EXTERN char* ntp_optarg;
MAR_EXTERN int ntp_optind;
MAR_EXTERN int ntp_optopt;
MAR_EXTERN int ntp_opterr;

MAR_EXTERN int
ntp_getopt(int argc, char** argv, const char* optstring);

#define aug_optarg ntp_optarg
#define aug_optind ntp_optind
#define aug_optopt ntp_optopt
#define aug_opterr ntp_opterr
#define aug_getopt ntp_getopt

#endif /* MAR_GETOPT_H */
