/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef MAR_CONFIG_H
#define MAR_CONFIG_H

#if !defined(__cplusplus)
# define MAR_EXTERN extern
#else /* __cplusplus */
# define MAR_EXTERN extern "C"
#endif /* __cplusplus */

#if HAVE_CONFIG_H
# ifndef AUGCONFIG_H
# define AUGCONFIG_H
#  include "augconfig.h"
# endif /* AUGCONFIG_H */
#endif /* HAVE_CONFIG_H */

#endif /* MAR_CONFIG_H */
