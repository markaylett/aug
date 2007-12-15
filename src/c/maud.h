/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef MAUD_H
#define MAUD_H

#include <stdarg.h>    /* va_list */
#include <stdlib.h>    /* NULL */
#include <sys/types.h> /* size_t */

#if !defined(__cplusplus)
# define MAUD_EXTERNC extern
#else /* __cplusplus */
# define MAUD_EXTERNC extern "C"
#endif /* __cplusplus */

#if defined(__CYGWIN__) || defined(__MINGW32__)
# define MAUD_EXPORT __attribute__ ((dllexport))
# define MAUD_IMPORT __attribute__ ((dllimport))
#elif defined(_MSC_VER)
# define MAUD_EXPORT __declspec(dllexport)
# define MAUD_IMPORT __declspec(dllimport)
#else /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */
# define MAUD_EXPORT
# define MAUD_IMPORT
#endif /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */

#if !defined(MAUD_BUILD)
# define MAUD_API MAUD_EXTERNC MAUD_IMPORT
#else /* MAUD_BUILD */
# define MAUD_API MAUD_EXTERNC MAUD_EXPORT
#endif /* MAUD_BUILD */

struct aug_blob_;
struct aug_object_;

enum maud_loglevel {
    MAUD_LOGCRIT,
    MAUD_LOGERROR,
    MAUD_LOGWARN,
    MAUD_LOGNOTICE,
    MAUD_LOGINFO,
    MAUD_LOGDEBUG
};

/**
   \defgroup TimerFlags Timer Flags
   \{
*/

/**
   Read timer.
*/

#define MAUD_TIMRD    0x01

/**
   Write timer.
*/

#define MAUD_TIMWR    0x02

/**
   Both read and write timer.
*/

#define MAUD_TIMRDWR (MAUD_TIMRD | MAUD_TIMWR)

/**
   \}
*/

/**
   \defgroup ShutFlags Shut Flags
   \{
*/

/**
   Force immediate shutdown.
*/

#define MAUD_SHUTNOW  0x01

/**
   \}
*/

/**
   \defgroup ReturnCodes Return Codes
   \{
*/


/**
   Success.
*/

#define MAUD_OK      0

/**
   Failure.
*/

#define MAUD_ERROR (-1)

/**
   None, empty or null depending on context.
*/

#define MAUD_NONE  (-2)

/**
   /}
*/

#define MAUD_MAXNAME     63

typedef int maud_id;

struct maud_session {
    char name_[MAUD_MAXNAME + 1];
    void* user_;
};

/**
   Both sockets are timers are represented by objects.
*/

struct maud_handle {
    maud_id id_;
    void* user_;
};

struct maud_host {

    /**
       The following functions are thread-safe.
    */

    /**
       Write message to the runtime log.

       \param level The log level.

       \param format Printf-style specification.

       \param ... Arguments to format specification.

       \sa #maud_loglevel, vwritelog_().
    */

    void (*writelog_)(int level, const char* format, ...);

    /**
       Write message to the runtime log.

       \param level The log level.

       \param format Printf-style specification.

       \param ... Arguments to format specification.

       \sa #maud_loglevel, writelog_().
    */

    void (*vwritelog_)(int level, const char* format, va_list args);

    /**
       Get a description of the last host error.

       If the return values of any host function indicates failure, this
       function can be used to obtain a description of the error.

       \return The error description.
    */

    const char* (*error_)(void);

    /**
       Re-configure the host and all loaded modules.

       \sa stopall_().
    */

    int (*reconfall_)(void);

    /**
       Stop the host environment.

       \sa reconfall_().
    */

    int (*stopall_)(void);

    /**
       Post an event to the event queue.

       \param to Target session name.

       \param type Type name associated with "ob".

       \param ob Optional object data.

       \sa dispatch_()
    */

    int (*post_)(const char* to, const char* type, struct aug_object_* ob);

    /**
       The remaining functions are not thread-safe.
    */

    /**
       Dispatch event to peer session.

       \param to Target session name.

       \param type Type name associated with "ob".

       \param ob Optional object data.

       \sa post_()
    */

    int (*dispatch_)(const char* to, const char* type,
                     struct aug_object_* ob);

    /**
       Read a configuration value.

       If the specified value does not exist in the configuration file, an
       attempt will be made to read the value from the environment table.

       \param name Name or key of desired value.

       \param def Optional default value to be returned.

       \return The default value if no value exists for name.
    */

    const char* (*getenv_)(const char* name, const char* def);

    /**
       Get the active session.
    */

    const struct maud_session* (*getsession_)(void);

    /**
       Shutdown the connection.

       \param cid Connection id.

       \param flags Use #MAUD_SHUTNOW to force immediate closure of the
       connection - do not wait for pending writes.
    */

    int (*shutdown_)(maud_id cid, unsigned flags);

    /**
       Establish tcp connection.

       This function will always return with the connection-id before the
       module is notified of connection establishment.

       \param host Ip address or host name.

       \param port Port or session name.

       \param user Optional user data to be associated with the resulting
       connection.

       \return The connection id.
    */

    int (*tcpconnect_)(const char* host, const char* port, void* user);

    /**
       Bind tcp listener socket.

       \param host Host to be bound.

       \param port Port to be bound.

       \param user Optional user data.
    */

    int (*tcplisten_)(const char* host, const char* port, void* user);

    /**
       Send data to peer.

       Data may be written to a client connection that has not been fully
       established.  In which case, the data will be buffered for writing once
       the connection has been established.

       \param cid Connection id.

       \param buf Data buffer.

       \param len Length of data buffer.
    */

    int (*send_)(maud_id cid, const void* buf, size_t len);

    /**
       Send data to peer.

       \param cid Connection id.

       \param blob Blob data.
    */

    int (*sendv_)(maud_id cid, struct aug_blob_* blob);

    /**
       Set read/write timer.

       \param cid Connection id.

       \param ms Timeout value in milliseconds.

       \param flags \ref TimerFlags.
    */

    int (*setrwtimer_)(maud_id cid, unsigned ms, unsigned flags);

    /**
       Reset read/write timer.

       \param cid Connection id.

       \param ms Timeout value in milliseconds.

       \param flags \ref TimerFlags.
    */

    int (*resetrwtimer_)(maud_id cid, unsigned ms, unsigned flags);

    /**
       Cancel read/write timer.

       \param cid Connection id.

       \param flags \ref TimerFlags.
    */

    int (*cancelrwtimer_)(maud_id cid, unsigned flags);


    /**
       Create new timer.

       \param ms Timeout value in milliseconds.

       \param ob Optional object data.
    */

    int (*settimer_)(unsigned ms, struct aug_object_* ob);

    /**
       Reset timer.

       \param tid Timer id.

       \param ms Timeout value in milliseconds.
    */

    int (*resettimer_)(maud_id tid, unsigned ms);

    /**
       Cancel timer.

       \param tid Timer id.
    */

    int (*canceltimer_)(maud_id tid);

    /**
       Set ssl client.

       \param cid Connection id.

       \param ctx SSL context.
    */

    int (*setsslclient_)(maud_id cid, const char* ctx);

    /**
       Set ssl server.

       \param cid Connection id.

       \param ctx SSL context.
    */

    int (*setsslserver_)(maud_id cid, const char* ctx);
};

/**
   Module functions of type int should return either #MAUD_OK or
   #MAUD_ERROR, depending on the result.  For those functions associated
   with a connection, a failure will result in the connection being closed.
*/

struct maud_module {

    /**
       Stop session.

       The current session can be retrieved using getsession_().  All
       resources associated with the session should be released in this
       handler.  stop_() will only be called for a session if start_()
       returned #MAUD_OK.
    */

    void (*stop_)(void);

    /**
       Start session.

       User-state associated with the session may be assigned to
       "session->user_".

       \return either #MAUD_OK or #MAUD_ERROR.
    */

    int (*start_)(struct maud_session* session);

    /**
       Re-configure request.

       Called in response to a #AUG_EVENTRECONF event, which are raise in
       response to either a #SIGHUP, or a call to reconfall_().
    */

    void (*reconf_)(void);

    /**
       Custom event notification.

       \param from Source session name.

       \param type Event type.

       \param ob Object data.
    */

    void (*event_)(const char* from, const char* type,
                   struct aug_object_* ob);

    /**
       Connection closure.

       \param sock The closed socket.
    */

    void (*closed_)(const struct maud_handle* sock);

    /**
       Teardown request.

       \param sock TODO
    */

    void (*teardown_)(const struct maud_handle* sock);

    /**
       Acceptance of socket connection.

       This function is called when a new connection is accepted on a listener
       socket.

       \param sock TODO

       \param addr TODO

       \param port TODO

       \return either #MAUD_OK or #MAUD_ERROR.
    */

    int (*accepted_)(struct maud_handle* sock, const char* addr,
                     unsigned short port);

    /**
       Completion of client connection handshake.

       This function is called when a connection, initiated by a call to
       tcpconnect_(), becomes established.

       \param sock TODO

       \param addr TODO

       \param port TODO

       \sa tcpconnect_()
    */

    void (*connected_)(struct maud_handle* sock, const char* addr,
                       unsigned short port);

    /**
       Inbound data.

       \param sock The socket on which the data was received.

       \param buf Data buffer.  May not be null terminated.

       \param len Length of data buffer.
    */

    void (*data_)(const struct maud_handle* sock, const void* buf,
                  size_t len);

    /**
       Expiry of read timer.

       \param sock TODO

       \param ms The current timeout value.  The callee may modify "ms" to
       specify a new value; a value of zero will cancel the timer.
    */

    void (*rdexpire_)(const struct maud_handle* sock, unsigned* ms);

    /**
       Expiry of write timer.

       \param sock TODO

       \param ms The current timeout value.  The callee may modify "ms" to
       specify a new value; a value of zero will cancel the timer.
    */

    void (*wrexpire_)(const struct maud_handle* sock, unsigned* ms);

    /**
       Timer expiry.

       \param timer TODO

       \param ms The current timeout value.  The callee may modify "ms" to
       specify a new value; a value of zero will cancel the timer.
    */

    void (*expire_)(const struct maud_handle* timer, unsigned* ms);

    /**
       Authorisation of peer certificate.

       \param sock TODO

       \param subject TODO

       \param issuer TODO

       \return either #MAUD_OK or #MAUD_ERROR.
    */

    int (*authcert_)(const struct maud_handle* sock, const char* subject,
                     const char* issuer);
};

MAUD_EXTERNC const struct maud_host*
maud_gethost(void);

/**
   Syntactic sugar that allows host functions to be called with a
   function-like syntax.
*/

#define maud_writelog      (maud_gethost()->writelog_)
#define maud_vwritelog     (maud_gethost()->vwritelog_)
#define maud_error         (maud_gethost()->error_)
#define maud_reconfall     (maud_gethost()->reconfall_)
#define maud_stopall       (maud_gethost()->stopall_)
#define maud_post          (maud_gethost()->post_)
#define maud_dispatch      (maud_gethost()->dispatch_)
#define maud_getenv        (maud_gethost()->getenv_)
#define maud_getsession    (maud_gethost()->getsession_)
#define maud_shutdown      (maud_gethost()->shutdown_)
#define maud_tcpconnect    (maud_gethost()->tcpconnect_)
#define maud_tcplisten     (maud_gethost()->tcplisten_)
#define maud_send          (maud_gethost()->send_)
#define maud_sendv         (maud_gethost()->sendv_)
#define maud_setrwtimer    (maud_gethost()->setrwtimer_)
#define maud_resetrwtimer  (maud_gethost()->resetrwtimer_)
#define maud_cancelrwtimer (maud_gethost()->cancelrwtimer_)
#define maud_settimer      (maud_gethost()->settimer_)
#define maud_resettimer    (maud_gethost()->resettimer_)
#define maud_canceltimer   (maud_gethost()->canceltimer_)
#define maud_setsslclient  (maud_gethost()->setsslclient_)
#define maud_setsslserver  (maud_gethost()->setsslserver_)

/**
   This macro defines the module's entry points.  maud_init() should return
   NULL on failure.
*/

#define MAUD_ENTRYPOINTS(init, term)                                \
    static const struct maud_host* host_ = NULL;                    \
    MAUD_EXTERNC const struct maud_host*                            \
    maud_gethost(void)                                              \
    {                                                               \
        return host_;                                               \
    }                                                               \
    MAUD_API const struct maud_module*                              \
    maud_init(const char* name, const struct maud_host* host)       \
    {                                                               \
        if (host_)                                                  \
            return NULL;                                            \
        host_ = host;                                               \
        return init(name);                                          \
    }                                                               \
    MAUD_API void                                                   \
    maud_term(void)                                                 \
    {                                                               \
        term();                                                     \
        host_ = NULL;                                               \
    }

typedef void (*maud_termfn)(void);
typedef const struct maud_module*
(*maud_initfn)(const char*, const struct maud_host*);

#endif /* MAUD_H */
