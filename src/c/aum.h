/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUM_H
#define AUM_H

/**
 * @file aum.h
 *
 * Application server modules.
 */

#include <stdarg.h>    /* va_list */
#include <stdlib.h>    /* NULL */
#include <sys/types.h> /* size_t */

#if !defined(__cplusplus)
# define AUM_EXTERNC extern
#else /* __cplusplus */
# define AUM_EXTERNC extern "C"
#endif /* __cplusplus */

#if defined(__CYGWIN__) || defined(__MINGW32__)
# define AUM_EXPORT __attribute__ ((dllexport))
# define AUM_IMPORT __attribute__ ((dllimport))
#elif defined(_MSC_VER)
# define AUM_EXPORT __declspec(dllexport)
# define AUM_IMPORT __declspec(dllimport)
#else /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */
# define AUM_EXPORT
# define AUM_IMPORT
#endif /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */

#if !defined(AUM_BUILD)
# define AUM_API AUM_EXTERNC AUM_IMPORT
#else /* AUM_BUILD */
# define AUM_API AUM_EXTERNC AUM_EXPORT
#endif /* AUM_BUILD */

struct aub_object_;
struct aug_blob_;

/**
 * @defgroup Module Module
 */

/**
 * @defgroup ModuleLogLevel Log Level
 *
 * @ingroup Module
 *
 * @see writelog_(), vwritelog_().
 */

enum aum_loglevel {
    AUM_LOGCRIT,
    AUM_LOGERROR,
    AUM_LOGWARN,
    AUM_LOGNOTICE,
    AUM_LOGINFO,
    AUM_LOGDEBUG
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

#define AUM_TIMRD    0x01

/**
 * Write timer.
 */

#define AUM_TIMWR    0x02

/**
 * Both read and write timer.
 */

#define AUM_TIMRDWR (AUM_TIMRD | AUM_TIMWR)

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

#define AUM_SHUTNOW  0x01

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

#define AUM_OK      0

/**
 * Failure.
 */

#define AUM_ERROR (-1)

/**
 * None, empty or null depending on context.
 */

#define AUM_NONE  (-2)

/** @} */

#define AUM_MAXNAME     63

typedef int aum_id;

struct aum_session {
    char name_[AUM_MAXNAME + 1];
    void* user_;
};

/**
 * Both sockets and timers are represented by handles.  For timer handles,
 * "user_" will be of type @ref aub_object.
 *
 * @see settimer_()
 */

struct aum_handle {
    aum_id id_;
    void* user_;
};

/**
 * @defgroup ModuleHost Host
 *
 * @ingroup Module
 *
 * @{
 */

struct aum_host {

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
     * @see #aum_loglevel, vwritelog_().
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
     * @see #aum_loglevel, writelog_().
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

    int (*post_)(const char* to, const char* type, struct aub_object_* ob);

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
                     struct aub_object_* ob);

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

    const struct aum_session* (*getsession_)(void);

    /**
     * Shutdown the connection.
     *
     * @param cid Connection id.
     *
     * @param flags Use #AUM_SHUTNOW to force immediate closure of the
     * connection - do not wait for pending writes.
     */

    int (*shutdown_)(aum_id cid, unsigned flags);

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
     * @param user Optional user data to be associated with the resulting
     * connection.
     *
     * @return The connection id.
     */

    int (*tcpconnect_)(const char* host, const char* port, void* user);

    /**
     * Bind tcp listener socket.
     *
     * @param host Host to be bound.
     *
     * @param port Port to be bound.
     *
     * @param user Optional user data.
     */

    int (*tcplisten_)(const char* host, const char* port, void* user);

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

    int (*send_)(aum_id cid, const void* buf, size_t len);

    /**
     * Send data to peer.
     *
     * @param cid Connection id.
     *
     * @param blob Blob data.
     */

    int (*sendv_)(aum_id cid, struct aug_blob_* blob);

    /**
     * Set read/write timer.
     *
     * @param cid Connection id.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    int (*setrwtimer_)(aum_id cid, unsigned ms, unsigned flags);

    /**
     * Reset read/write timer.
     *
     * @param cid Connection id.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    int (*resetrwtimer_)(aum_id cid, unsigned ms, unsigned flags);

    /**
     * Cancel read/write timer.
     *
     * @param cid Connection id.
     *
     * @param flags @ref ModuleTimerFlags.
     */

    int (*cancelrwtimer_)(aum_id cid, unsigned flags);

    /**
     * Create new timer.
     *
     * @param ms Timeout value in milliseconds.
     *
     * @param ob Optional object data.
     */

    int (*settimer_)(unsigned ms, struct aub_object_* ob);

    /**
     * Reset timer.
     *
     * @param tid Timer id.
     *
     * @param ms Timeout value in milliseconds.
     */

    int (*resettimer_)(aum_id tid, unsigned ms);

    /**
     * Cancel timer.
     *
     * @param tid Timer id.
     */

    int (*canceltimer_)(aum_id tid);

    /**
     * Set ssl client.
     *
     * @param cid Connection id.
     *
     * @param ctx SSL context.
     */

    int (*setsslclient_)(aum_id cid, const char* ctx);

    /**
     * Set ssl server.
     *
     * @param cid Connection id.
     *
     * @param ctx SSL context.
     */

    int (*setsslserver_)(aum_id cid, const char* ctx);
};

/** @} */

/**
 * @addtogroup Module
 *
 * Module functions of type int should return either #AUM_OK or #AUM_ERROR,
 * depending on the result.  For those functions associated with a connection,
 * a failure will result in the connection being closed.
 *
 * @{
 */

struct aum_module {

    /**
     * Stop session.
     *
     * The current session can be retrieved using aum_host::getsession_().
     * All resources associated with the session should be released in this
     * handler.  stop_() will only be called for a session if start_()
     * returned #AUM_OK.
     */

    void (*stop_)(void);

    /**
     * Start session.
     *
     * User-state associated with the session may be assigned to
     * #aum_session::user_.
     *
     * @return Either #AUM_OK or #AUM_ERROR.
     */

    int (*start_)(struct aum_session* session);

    /**
     * Re-configure request.
     *
     * Called in response to a #AUG_EVENTRECONF event, which are raise in
     * response to either a #SIGHUP, or a call to aum_host::reconfall_().
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
                   struct aub_object_* ob);

    /**
     * Connection closure.
     *
     * @param sock The closed socket.
     */

    void (*closed_)(const struct aum_handle* sock);

    /**
     * Teardown request.
     *
     * @param sock Socket descriptor.
     */

    void (*teardown_)(const struct aum_handle* sock);

    /**
     * Acceptance of socket connection.
     *
     * This function is called when a new connection is accepted on a listener
     * socket.
     *
     * @param sock Socket descriptor.
     *
     * @param addr Peer address.
     *
     * @param port Peer port.
     *
     * @return Either #AUM_OK or #AUM_ERROR.
     */

    int (*accepted_)(struct aum_handle* sock, const char* addr,
                     unsigned short port);

    /**
     * Completion of client connection handshake.
     *
     * This function is called when a connection, initiated by a call to
     * aum_host::tcpconnect_(), becomes established.
     *
     * @param sock Socket descriptor.
     *
     * @param addr Peer address.
     *
     * @param port Peer port.
     *
     * @see aum_host::tcpconnect_().
     */

    void (*connected_)(struct aum_handle* sock, const char* addr,
                       unsigned short port);

    /**
     * Inbound data.
     *
     * @param sock The socket on which the data was received.
     *
     * @param buf Data buffer.  May not be null terminated.
     *
     * @param len Length of data buffer.
     */

    void (*data_)(const struct aum_handle* sock, const void* buf,
                  size_t len);

    /**
     * Expiry of read timer.
     *
     * @param sock Socket descriptor.
     *
     * @param ms The current timeout value.  The callee may modify @a ms to
     * specify a new value; a value of zero will cancel the timer.
     */

    void (*rdexpire_)(const struct aum_handle* sock, unsigned* ms);

    /**
     * Expiry of write timer.
     *
     * @param sock Socket descriptor.
     *
     * @param ms The current timeout value.  The callee may modify @a ms to
     * specify a new value; a value of zero will cancel the timer.
     */

    void (*wrexpire_)(const struct aum_handle* sock, unsigned* ms);

    /**
     * Timer expiry.
     *
     * @param timer Timer handle.
     *
     * @param ms The current timeout value.  The callee may modify @a ms to
     * specify a new value; a value of zero will cancel the timer.
     */

    void (*expire_)(const struct aum_handle* timer, unsigned* ms);

    /**
     * Authorisation of peer certificate.
     *
     * @param sock Socket descriptor.
     *
     * @param subject Certificate subject.
     *
     * @param issuer Certificate issuer.
     *
     * @return Either #AUM_OK or #AUM_ERROR.
     */

    int (*authcert_)(const struct aum_handle* sock, const char* subject,
                     const char* issuer);
};

/** @} */

AUM_EXTERNC const struct aum_host*
aum_gethost(void);

/**
 * Syntactic sugar that allows host functions to be called with a
 * function-like syntax.
 */

#define aum_writelog      (aum_gethost()->writelog_)
#define aum_vwritelog     (aum_gethost()->vwritelog_)
#define aum_error         (aum_gethost()->error_)
#define aum_reconfall     (aum_gethost()->reconfall_)
#define aum_stopall       (aum_gethost()->stopall_)
#define aum_post          (aum_gethost()->post_)
#define aum_dispatch      (aum_gethost()->dispatch_)
#define aum_getenv        (aum_gethost()->getenv_)
#define aum_getsession    (aum_gethost()->getsession_)
#define aum_shutdown      (aum_gethost()->shutdown_)
#define aum_tcpconnect    (aum_gethost()->tcpconnect_)
#define aum_tcplisten     (aum_gethost()->tcplisten_)
#define aum_send          (aum_gethost()->send_)
#define aum_sendv         (aum_gethost()->sendv_)
#define aum_setrwtimer    (aum_gethost()->setrwtimer_)
#define aum_resetrwtimer  (aum_gethost()->resetrwtimer_)
#define aum_cancelrwtimer (aum_gethost()->cancelrwtimer_)
#define aum_settimer      (aum_gethost()->settimer_)
#define aum_resettimer    (aum_gethost()->resettimer_)
#define aum_canceltimer   (aum_gethost()->canceltimer_)
#define aum_setsslclient  (aum_gethost()->setsslclient_)
#define aum_setsslserver  (aum_gethost()->setsslserver_)

/**
 * This macro defines the module's entry points.  aum_init() should return
 * NULL on failure.
 */

#define AUM_ENTRYPOINTS(init, term)                                 \
    static const struct aum_host* host_ = NULL;                     \
    AUM_EXTERNC const struct aum_host*                              \
    aum_gethost(void)                                               \
    {                                                               \
        return host_;                                               \
    }                                                               \
    AUM_API const struct aum_module*                                \
    aum_init(const char* name, const struct aum_host* host)         \
    {                                                               \
        if (host_)                                                  \
            return NULL;                                            \
        host_ = host;                                               \
        return init(name);                                          \
    }                                                               \
    AUM_API void                                                    \
    aum_term(void)                                                  \
    {                                                               \
        term();                                                     \
        host_ = NULL;                                               \
    }

typedef void (*aum_termfn)(void);
typedef const struct aum_module*
(*aum_initfn)(const char*, const struct aum_host*);

#endif /* AUM_H */
