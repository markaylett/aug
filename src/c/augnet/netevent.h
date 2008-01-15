/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_NETEVENT_H
#define AUGNET_NETEVENT_H

/**
 * @file augnet/netevent.h
 *
 * Network event packet format.
 */

#include "augnet/config.h"

#include "augsys/socket.h" /* AUG_MAXHOSTSERVLEN */

/**
 * @defgroup NetEvent NetEvent
 */

/**
 * @defgroup NetEventLimits NetEvent Limits
 *
 * @ingroup NetEvent
 *
 * @{
 */

#define AUG_NEVPROTO_MAX  UINT32_MAX
#define AUG_NEVNAMELEN    128
#define AUG_NEVADDRLEN    AUG_MAXHOSTSERVLEN
#define AUG_NEVTYPE_MAX   UINT32_MAX
#define AUG_NEVSTATE_MAX  UINT32_MAX
#define AUG_NEVSEQ_MAX    UINT32_MAX
#define AUG_NEVHBSEC_MAX  UINT8_MAX
#define AUG_NEVWEIGHT_MAX UINT8_MAX
#define AUG_NEVLOAD_MAX   UINT8_MAX
#define AUG_NEVRESV_MAX   UINT8_MAX

/** @} */

/**
 * @defgroup NetEventOffsets NetEvent Offsets
 *
 * @ingroup NetEvent
 *
 * @{
 */

#define AUG_NEVPROTO_OFFSET   0
#define AUG_NEVNAME_OFFSET   (AUG_NEVPROTO_OFFSET + sizeof(uint32_t))
#define AUG_NEVADDR_OFFSET   (AUG_NEVNAME_OFFSET + AUG_NEVNAMELEN)
#define AUG_NEVTYPE_OFFSET   (AUG_NEVADDR_OFFSET + AUG_NEVADDRLEN)
#define AUG_NEVSTATE_OFFSET  (AUG_NEVTYPE_OFFSET + sizeof(uint32_t))
#define AUG_NEVSEQ_OFFSET    (AUG_NEVSTATE_OFFSET + sizeof(uint32_t))
#define AUG_NEVHBSEC_OFFSET  (AUG_NEVSEQ_OFFSET + sizeof(uint32_t))
#define AUG_NEVWEIGHT_OFFSET (AUG_NEVHBSEC_OFFSET + sizeof(uint8_t))
#define AUG_NEVLOAD_OFFSET   (AUG_NEVWEIGHT_OFFSET + sizeof(uint8_t))
#define AUG_NEVRESV_OFFSET   (AUG_NEVLOAD_OFFSET + sizeof(uint8_t))

#define AUG_NETEVENT_SIZE    (AUG_NEVRESV_OFFSET + sizeof(uint8_t))

/** @} */

/**
 * The #aug_netevent::type_ value for heartbeats.
 */

#define AUG_NEVHEARTBEAT 1

/**
 * NetEvent packet structure.
 */

struct aug_netevent {
    unsigned proto_;
    char name_[AUG_NEVNAMELEN + 1];
    char addr_[AUG_NEVADDRLEN + 1];
    unsigned type_, state_, seq_, hbsec_, weight_, load_;
};

/**
 * Verify that netevent elements do not exceed limits.
 *
 * @param event NetEvent object.
 *
 * @return -1 on error.
 */

AUGNET_API int
aug_verifynetevent(const struct aug_netevent* event);

/**
 * Serialise netevent packet to @a buf.
 *
 * @param buf Output buffer.  Must be at least #AUG_NETEVENT_SIZE bytes in
 * length.
 *
 * @param event NetEvent object.
 *
 * @return @a buf, or NULL on error.
 */

AUGNET_API char*
aug_packnetevent(char* buf, const struct aug_netevent* event);

/**
 * De-serialise netevent packet to @a event.
 *
 * @param event Output netevent object.
 *
 * @param buf Input buffer.
 *
 * @return @a event, or NULL on error.
 */

AUGNET_API struct aug_netevent*
aug_unpacknetevent(struct aug_netevent* event, const char* buf);

#endif /* AUGNET_NETEVENT_H */
