/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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
 * @defgroup ModuleReturnCodes Return Codes
 *
 * @ingroup Module
 *
 * @{
 */

/**
 * Success.
 */

#define MOD_OK      0

/**
 * Failure.
 */

#define MOD_ERROR (-1)

/**
 * None, empty or null depending on context.
 */

#define MOD_NONE  (-2)

/** @} */

#define MOD_MAXNAME     63

typedef unsigned mod_id;

struct mod_session {
    char name_[MOD_MAXNAME + 1];
    void* user_;
};

/**
 * Both sockets and timers are represented by handles.  For timer handles,
 * #mod_handle::user_ will be of type @ref aug_object.
 *
 * @see settimer_()
 */

struct mod_handle {
    mod_id id_;
    void* user_;
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

    const char* (*error_)(void);

    /**
     * Re-configure the host and all loaded modules.
     *
     * @see stopall_().
     */

    int (*reconfall_)(void);

    /**
     * Stop the host environment.
     *
     * @see reconfall_().
     */

    int (*stopall_)(void);

    /**
     * Post an event to the event queue.
     *
     * @param to Target session name.
     *
     * @param type Event type associated with @a ob.
     *
     * @param ob Optional object data.
     *
     * @see dispatch_()
     */

    int (*post_)(const char* to, const char* type, struct aug_object_* ob);

    /**
     * The remaining functions are not thread-safe.
     */

    /**
     * Dispatch event to peer session.
     *
     * @param to Target session name.
     *
     * @param type Event type associated with @a ob.
     *
     * @param ob Optional object data.
     *
     * @see post_()
     */

    int (*dispatch_)(const char* to, const char* type,
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
     * Get the active session.
     */

    const struct mod_session* (*getsession_)(void);

    /**
     * Shutdown the connection.
     *
     * @param cid Connection id.
     *
     * @param flags Use #MOD_SHUTNOW to force immediate closure of the
     * connection - do not wait for pending writes.
     */

    int (*shutdown_)(mod_id cid, unsigned flags);

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
     * @param user Optional user data to be associated with the resulting
     * connection.
     *
     * @return The connection id.
     */

    int (*tcpconnect_)(const char* host, const char* port, const char* sslctx,
                       void* user);

    /**
     * Bind tcp listener socket.
     *
     * @param host Host to be bound.
     *
     * @param port Port to be bound.
     *
     * @param sslctx Optional name of ssl context.
     *
     * @param user Optional user data.
     */

    int (*tcplisten_)(const char* host, const char* port, const char* sslctx,
                      void* user);

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

    int (*send_)(mod_id cid, const void* buf, size_t len);

    /**
     * Send data to peer.
     *
     * @param cid Connection id.
     *
     * @param blob Blob data.
     */

    int (*sendv_)(mod_id cid, struct aug_blob_* blob);

    /**
     * Set read/write timer.
     *
     * @param cid Connection id.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    int (*setrwtimer_)(mod_id cid, unsigned ms, unsigned flags);

    /**
     * Reset read/write timer.
     *
     * @param cid Connection id.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    int (*resetrwtimer_)(mod_id cid, unsigned ms, unsigned flags);

    /**
     * Cancel read/write timer.
     *
     * @param cid Connection id.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    int (*cancelrwtimer_)(mod_id cid, unsigned flags);

    /**
     * Create new timer.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param ob Optional object data.
     */

    int (*settimer_)(unsigned ms, struct aug_object_* ob);

    /**
     * Reset timer.
     *
     * @param tid Timer id.
     *
     * @param ms Timeout value in milliseconds.
     */

    int (*resettimer_)(mod_id tid, unsigned ms);

    /**
     * Cancel timer.
     *
     * @param tid Timer id.
     */

    int (*canceltimer_)(mod_id tid);
};

/** @} */

/**
 * @addtogroup Module
 *
 * Module functions of type int should return either #MOD_OK or #MOD_ERROR,
 * depending on the result.  For those functions associated with a connection,
 * a failure will result in the connection being closed.
 *
 * @{
 */

struct mod_module {

    /**
     * Stop session.
     *
     * The current session can be retrieved using mod_host::getsession_().
     * All resources associated with the session should be released in this
     * handler.  stop_() will only be called for a session if start_()
     * returned #MOD_OK.
     */

    void (*stop_)(void);

    /**
     * Start session.
     *
     * User-state associated with the session may be assigned to
     * #mod_session::user_.
     *
     * @return Either #MOD_OK or #MOD_ERROR.
     */

    int (*start_)(struct mod_session* session);

    /**
     * Re-configure request.
     *
     * Called in response to a #AUG_EVENTRECONF event, which are raise in
     * response to either a #SIGHUP, or a call to mod_host::reconfall_().
     */

    void (*reconf_)(void);

    /**
     * Custom event notification.
     *
     * @param from Source session name.
     *
     * @param type Event type.
     *
     * @param ob Object data.
     */

    void (*event_)(const char* from, const char* type,
                   struct aug_object_* ob);

    /**
     * Connection closure.
     *
     * @param sock The closed socket.
     */

    void (*closed_)(const struct mod_handle* sock);

    /**
     * Teardown request.
     *
     * @param sock Socket descriptor.
     */

    void (*teardown_)(const struct mod_handle* sock);

    /**
     * Acceptance of socket connection.
     *
     * This function is called when a new connection is accepted on a listener
     * socket.
     *
     * @param sock Socket descriptor.
     *
     * @param name Peer address.
     *
     * @return Either #MOD_OK or #MOD_ERROR.
     */

    int (*accepted_)(struct mod_handle* sock, const char* name);

    /**
     * Completion of client connection handshake.
     *
     * This function is called when a connection, initiated by a call to
     * mod_host::tcpconnect_(), becomes established.
     *
     * @param sock Socket descriptor.
     *
     * @param name Peer address.
     *
     * @see mod_host::tcpconnect_().
     */

    void (*connected_)(struct mod_handle* sock, const char* name);

    /**
     * Inbound data.
     *
     * @param sock The socket on which the data was received.
     *
     * @param buf Data buffer.  May not be null terminated.
     *
     * @param len Length of data buffer.
     */

    void (*data_)(const struct mod_handle* sock, const void* buf,
                  size_t len);

    /**
     * Expiry of read timer.
     *
     * @param sock Socket descriptor.
     *
     * @param ms The current timeout value.  The callee may modify @a ms to
     * specify a new value; a value of zero will cancel the timer.
     */

    void (*rdexpire_)(const struct mod_handle* sock, unsigned* ms);

    /**
     * Expiry of write timer.
     *
     * @param sock Socket descriptor.
     *
     * @param ms The current timeout value.  The callee may modify @a ms to
     * specify a new value; a value of zero will cancel the timer.
     */

    void (*wrexpire_)(const struct mod_handle* sock, unsigned* ms);

    /**
     * Timer expiry.
     *
     * @param timer Timer handle.
     *
     * @param ms The current timeout value.  The callee may modify @a ms to
     * specify a new value; a value of zero will cancel the timer.
     */

    void (*expire_)(const struct mod_handle* timer, unsigned* ms);

    /**
     * Authorisation of peer certificate.
     *
     * @param sock Socket descriptor.
     *
     * @param subject Certificate subject.
     *
     * @param issuer Certificate issuer.
     *
     * @return Either #MOD_OK or #MOD_ERROR.
     */

    int (*authcert_)(const struct mod_handle* sock, const char* subject,
                     const char* issuer);
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
#define mod_error         (mod_gethost()->error_)
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

#define MOD_ENTRYPOINTS(init, term)                                 \
    static const struct mod_host* host_ = NULL;                     \
    MOD_EXTERNC const struct mod_host*                              \
    mod_gethost(void)                                               \
    {                                                               \
        return host_;                                               \
    }                                                               \
    MOD_API const struct mod_module*                                \
    mod_init(const char* name, const struct mod_host* host)         \
    {                                                               \
        if (host_)                                                  \
            return NULL;                                            \
        host_ = host;                                               \
        return init(name);                                          \
    }                                                               \
    MOD_API void                                                    \
    mod_term(void)                                                  \
    {                                                               \
        term();                                                     \
        host_ = NULL;                                               \
    }

typedef void (*mod_termfn)(void);
typedef const struct mod_module*
(*mod_initfn)(const char*, const struct mod_host*);

#endif /* AUGMOD_H */
