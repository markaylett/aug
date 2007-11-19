/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_NBFILE_H
#define AUGNET_NBFILE_H

#include "augnet/config.h"
#include "augnet/types.h"

struct timeval;

AUGNET_API aug_nbfiles_t
aug_createnbfiles(void);

AUGNET_API int
aug_destroynbfiles(aug_nbfiles_t nbfiles);

/**
   Once inserted, a call to aug_close() will remove the file from the nbfiles
   set.
*/

AUGNET_API int
aug_insertnbfile(aug_nbfiles_t nbfiles, int fd, aug_nbfilecb_t cb,
                 aug_object_t user);

/**
   Can be used to remove a file without closing it.
*/

AUGNET_API int
aug_removenbfile(int fd);

AUGNET_API int
aug_foreachnbfile(aug_nbfiles_t nbfiles);

AUGNET_API int
aug_emptynbfiles(aug_nbfiles_t nbfiles);

/**
   \returns a positive value if events are pending: there is no gaurantee this
   this value be representative of the actual number of events pending.
*/

AUGNET_API int
aug_waitnbevents(aug_nbfiles_t nbfiles, const struct timeval* timeout);

AUGNET_API int
aug_shutdownnbfile(int fd);

AUGNET_API int
aug_setnbeventmask(int fd, unsigned short mask);

AUGNET_API int
aug_nbeventmask(int fd);

AUGNET_API int
aug_nbevents(int fd);

#endif /* AUGNET_NBFILE_H */
