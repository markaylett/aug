/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_HEARTBEAT_H
#define AUGNET_HEARTBEAT_H

/**
 * @file augnet/heartbeat.h
 *
 * Heartbeat packet format.
 */

#include "augnet/config.h"

/**
 * @defgroup Heartbeat Heartbeat
 */

/**
 * @defgroup HeartbeatLimits Heartbeat Limits
 *
 * @ingroup Heartbeat
 *
 * @{
 */

#define AUG_HBHEAD_MAX  UINT32_MAX
#define AUG_HBNAMELEN   64
#define AUG_HBSEQ_MAX   UINT32_MAX
#define AUG_HBSTATE_MAX UINT32_MAX
#define AUG_HBLOAD_MAX  UINT8_MAX
#define AUG_HBSEC_MAX   UINT8_MAX
#define AUG_HBMSG_MAX   UINT16_MAX

/** @} */

/**
 * @defgroup HeartbeatOffsets Heartbeat Offsets
 *
 * @ingroup Heartbeat
 *
 * @{
 */

#define AUG_HBHEAD_OFFSET 0
#define AUG_HBNAME_OFFSET (AUG_HBHEAD_OFFSET + sizeof(uint32_t))
#define AUG_HBSEQ_OFFSET (AUG_HBNAME_OFFSET + AUG_HBNAMELEN)
#define AUG_HBSTATE_OFFSET (AUG_HBSEQ_OFFSET + sizeof(uint32_t))
#define AUG_HBLOAD_OFFSET (AUG_HBSTATE_OFFSET + sizeof(uint32_t))
#define AUG_HBSEC_OFFSET (AUG_HBLOAD_OFFSET + sizeof(uint8_t))
#define AUG_HBMSG_OFFSET (AUG_HBSEC_OFFSET + sizeof(uint8_t))
#define AUG_HEARTBEAT_SIZE (AUG_HBMSG_OFFSET + sizeof(uint16_t))

/** @} */

/**
 * Heartbeat packet structure.
 */

struct aug_heartbeat {
    unsigned head_;
    char name_[AUG_HBNAMELEN + 1];
    unsigned seq_, state_, load_, sec_, msg_;
};

/**
 * Verify that heartbeat elements do not exceed limits.
 *
 * @param hb Heartbeat object.
 *
 * @return -1 on error.
 */

AUGNET_API int
aug_verifyheartbeat(const struct aug_heartbeat* hb);

/**
 * Serialise heartbeat packet to @a buf.
 *
 * @param buf Output buffer.  Must be at least #AUG_HEARTBEAT_SIZE bytes in
 * length.
 *
 * @param hb Heartbeat object.
 *
 * @return @a buf, or NULL on error.
 */

AUGNET_API char*
aug_packheartbeat(char* buf, const struct aug_heartbeat* hb);

/**
 * De-serialise heartbeat packet to @a hb.
 *
 * @param hb Output heartbeat object.
 *
 * @param buf Input buffer.
 *
 * @return @a hb, or NULL on error.
 */

AUGNET_API struct aug_heartbeat*
aug_unpackheartbeat(struct aug_heartbeat* hb, const char* buf);

#endif /* AUGNET_HEARTBEAT_H */
