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
#ifndef AUGMOD_H
#define AUGMOD_H

/**
 * @file augmod.h
 *
 * Application server modules.
 *
 * Defines the contract between application servers and modules.  Modules are
 * the dynamically loaded plugins that provide application server's with their
 * application specific behaviours.
 */

/**
 * @defgroup Module Module
 */

#include "augabi.h"

#include <stdarg.h> /* va_list */
#include <stdlib.h> /* NULL */

#define MOD_EXTERNC AUG_EXTERNC
#define MOD_EXPORT  AUG_EXPORT
#define MOD_IMPORT  AUG_IMPORT

#if !defined(MOD_BUILD)
# define MOD_API MOD_EXTERNC MOD_IMPORT
#else /* MOD_BUILD */
# define MOD_API MOD_EXTERNC MOD_EXPORT
#endif /* MOD_BUILD */

struct aug_blob_;
struct mod_session_;

/**
 * @defgroup ModuleLogLevel Log Level
 *
 * @ingroup Module
 *
 * @see writelog_(), vwritelog_().
 */

enum mod_loglevel {
    MOD_LOGCRIT,
    MOD_LOGERROR,
    MOD_LOGWARN,
    MOD_LOGNOTICE,
    MOD_LOGINFO,
    MOD_LOGDEBUG
};

/**
 * @defgroup ModuleTimerFlags Timer Flags
 *
 * @ingroup Module
 *
 * @{
 */

/**
 * Read timer.
 */

#define MOD_TIMRD    0x01

/**
 * Write timer.
 */

#define MOD_TIMWR    0x02

/**
 * Both read and write timer.
 */

#define MOD_TIMRDWR (MOD_TIMRD | MOD_TIMWR)

/** @} */

/**
 * @defgroup ModuleShutFlags Shut Flags
 *
 * @ingroup Module
 *
 * @{
 */

/**
 * Force immediate shutdown.
 */

#define MOD_SHUTNOW  0x01

/** @} */

/**
 * @defgroup ModuleBool Boolean Values
 *
 * @ingroup Module
 *
 * @{
 */

/**
 * Boolean type.
 *
 * C++ enums are not guaranteed to have sizeof(int) so int is used instead.
 *
 * Do not use #AUG_TRUE in tests as any non-zero value is considered true.
 *
 */

typedef int mod_bool;

#define MOD_FALSE 0
#define MOD_TRUE  1

/** @} */

/**
 * @defgroup ModuleResultCodes Result Codes
 *
 * @ingroup Module
 *
 * @{
 */

/**
 * Result type.
 */

typedef int mod_result;

/**
 * Integer result type.
 */

typedef int mod_rint;

/**
 * Success.
 */

#define MOD_SUCCESS     0

/**
 * Error.
 */

#define MOD_FAILERROR (-1)

/**
 * None, empty or null depending on context.
 */

#define MOD_FAILNONE  (-2)

/** @} */

#define MOD_MAXNAME     63

typedef unsigned mod_id;

/**
 * Both sockets and timers are represented by handles.  For timer handles,
 * #mod_handle::user_ will be of type @ref aug_object.
 *
 * @see settimer_()
 */

struct mod_handle {
    mod_id id_;
    aug_object* ob_;
};

/**
 * @defgroup ModuleHost Host
 *
 * @ingroup Module
 *
 * @{
 */

struct mod_host {

    /**
     * The following functions are thread-safe.
     */

    /**
     * Write message to the application server's log.
     *
     * @param level The log level.
     *
     * @param format Printf-style specification.
     *
     * @param ... Arguments to format specification.
     *
     * @see #mod_loglevel, vwritelog_().
     */

    void (*writelog_)(int level, const char* format, ...);

    /**
     * Write message to the application server's log.
     *
     * @param level The log level.
     *
     * @param format Printf-style specification.
     *
     * @param ... Arguments to format specification.
     *
     * @see #mod_loglevel, writelog_().
     */

    void (*vwritelog_)(int level, const char* format, va_list args);

    /**
     * Get description for the last host error.
     *
     * If the return values of any host function indicates failure, this
     * function can be used to obtain a description of the error.
     *
     * @return The error description.
     */

    const char* (*geterror_)(void);

    /**
     * Re-configure the host and all loaded modules.
     *
     * @see stopall_().
     */

    mod_result (*reconfall_)(void);

    /**
     * Stop the host environment.
     *
     * @see reconfall_().
     */

    mod_result (*stopall_)(void);

    /**
     * Post an event to the event queue.
     *
     * @param id Originating id.
     *
     * @param to Target session name.
     *
     * @param type Event type associated with @a ob.
     *
     * @param ob Optional object data.
     *
     * @see dispatch_()
     */

    mod_result (*post_)(const char* to, const char* type, mod_id id,
                        struct aug_object_* ob);

    /**
     * The remaining functions are not thread-safe.
     */

    /**
     * Dispatch event to peer session.
     *
     * @param id Originating id.
     *
     * @param to Target session name.
     *
     * @param type Event type associated with @a ob.
     *
     * @param ob Optional object data.
     *
     * @see post_()
     */

    mod_result (*dispatch_)(const char* to, const char* type, mod_id id,
                            struct aug_object_* ob);

    /**
     * Read a configuration value.
     *
     * If the specified value does not exist in the configuration file, an
     * attempt will be made to read the value from the environment table.
     *
     * @param name Name or key of desired value.
     *
     * @param def Optional default value to be returned.
     *
     * @return The default value if no value exists for name.
     */

    const char* (*getenv_)(const char* name, const char* def);

    /**
     * Shutdown the connection.
     *
     * @param cid Connection id.
     *
     * @param flags Use #MOD_SHUTNOW to force immediate closure of the
     * connection - do not wait for pending writes.
     */

    mod_result (*shutdown_)(mod_id cid, unsigned flags);

    /**
     * Establish tcp connection.
     *
     * This function will always return with the connection-id before the
     * module is notified of connection establishment.
     *
     * @param host Ip address or host name.
     *
     * @param port Port or session name.
     *
     * @param sslctx Optional name of ssl context.
     *
     * @param ob Optional user data to be associated with the resulting
     * connection.
     *
     * @return The connection id.
     */

    mod_rint (*tcpconnect_)(const char* host, const char* port,
                            const char* sslctx, aug_object* ob);

    /**
     * Bind tcp listener socket.
     *
     * @param host Host to be bound.
     *
     * @param port Port to be bound.
     *
     * @param sslctx Optional name of ssl context.
     *
     * @param ob Optional user data.
     *
     * @return The listener id.
     */

    mod_rint (*tcplisten_)(const char* host, const char* port,
                           const char* sslctx, aug_object* ob);

    /**
     * Send data to peer.
     *
     * Data may be written to a client connection that has not been fully
     * established.  In which case, the data will be buffered for writing once
     * the connection has been established.
     *
     * @param cid Connection id.
     *
     * @param buf Data buffer.
     *
     * @param len Length of data buffer.
     */

    mod_result (*send_)(mod_id cid, const void* buf, size_t len);

    /**
     * Send data to peer.
     *
     * @param cid Connection id.
     *
     * @param blob Blob data.
     */

    mod_result (*sendv_)(mod_id cid, struct aug_blob_* blob);

    /**
     * Set read/write timer.
     *
     * @param cid Connection id.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    mod_result (*setrwtimer_)(mod_id cid, unsigned ms, unsigned flags);

    /**
     * Reset read/write timer.
     *
     * @param cid Connection id.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    mod_result (*resetrwtimer_)(mod_id cid, unsigned ms, unsigned flags);

    /**
     * Cancel read/write timer.
     *
     * @param cid Connection id.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    mod_result (*cancelrwtimer_)(mod_id cid, unsigned flags);

    /**
     * Create new timer.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param ob Optional object data.
     */

    mod_rint (*settimer_)(unsigned ms, aug_object* ob);

    /**
     * Reset timer.
     *
     * @param tid Timer id.
     *
     * @param ms Timeout value in milliseconds.
     */

    mod_result (*resettimer_)(mod_id tid, unsigned ms);

    /**
     * Cancel timer.
     *
     * @param tid Timer id.
     */

    mod_result (*canceltimer_)(mod_id tid);
};

/** @} */

MOD_EXTERNC const struct mod_host*
mod_gethost(void);

/**
 * Syntactic sugar that allows host functions to be called with a
 * function-like syntax.
 */

#define mod_writelog      (mod_gethost()->writelog_)
#define mod_vwritelog     (mod_gethost()->vwritelog_)
#define mod_geterror      (mod_gethost()->geterror_)
#define mod_reconfall     (mod_gethost()->reconfall_)
#define mod_stopall       (mod_gethost()->stopall_)
#define mod_post          (mod_gethost()->post_)
#define mod_dispatch      (mod_gethost()->dispatch_)
#define mod_getenv        (mod_gethost()->getenv_)
#define mod_getsession    (mod_gethost()->getsession_)
#define mod_shutdown      (mod_gethost()->shutdown_)
#define mod_tcpconnect    (mod_gethost()->tcpconnect_)
#define mod_tcplisten     (mod_gethost()->tcplisten_)
#define mod_send          (mod_gethost()->send_)
#define mod_sendv         (mod_gethost()->sendv_)
#define mod_setrwtimer    (mod_gethost()->setrwtimer_)
#define mod_resetrwtimer  (mod_gethost()->resetrwtimer_)
#define mod_cancelrwtimer (mod_gethost()->cancelrwtimer_)
#define mod_settimer      (mod_gethost()->settimer_)
#define mod_resettimer    (mod_gethost()->resettimer_)
#define mod_canceltimer   (mod_gethost()->canceltimer_)

/**
 * This macro defines the module's entry points.  mod_init() should return
 * null on failure.
 */

#define MOD_ENTRYPOINTS(init, term, create)                         \
    static const struct mod_host* host_ = NULL;                     \
    MOD_EXTERNC const struct mod_host*                              \
    mod_gethost(void)                                               \
    {                                                               \
        return host_;                                               \
    }                                                               \
    MOD_API mod_bool                                                \
    mod_init(const char* name, const struct mod_host* host)         \
    {                                                               \
        if (host_)                                                  \
            return MOD_FALSE;                                       \
        host_ = host;                                               \
        return init(name);                                          \
    }                                                               \
    MOD_API void                                                    \
    mod_term(void)                                                  \
    {                                                               \
        term();                                                     \
        host_ = NULL;                                               \
    }                                                               \
    MOD_API struct mod_session_*                                    \
    mod_create(const char* name)                                    \
    {                                                               \
        return create(name);                                        \
    }                                                               \

typedef void (*mod_termfn)(void);
typedef mod_bool
(*mod_initfn)(const char*, const struct mod_host*);
typedef struct mod_session_*
(*mod_createfn)(const char*);

#endif /* AUGMOD_H */
