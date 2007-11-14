/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGMOD_H
#define AUGMOD_H

#include <stdarg.h>    /* va_list */
#include <stdlib.h>    /* NULL */
#include <sys/types.h> /* size_t */

#if !defined(__cplusplus)
# define AUGMOD_EXTERNC extern
#else /* __cplusplus */
# define AUGMOD_EXTERNC extern "C"
#endif /* __cplusplus */

#if defined(__CYGWIN__) || defined(__MINGW32__)
# define AUGMOD_EXPORT __attribute__ ((dllexport))
# define AUGMOD_IMPORT __attribute__ ((dllimport))
#elif defined(_MSC_VER)
# define AUGMOD_EXPORT __declspec(dllexport)
# define AUGMOD_IMPORT __declspec(dllimport)
#else /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */
# define AUGMOD_EXPORT
# define AUGMOD_IMPORT
#endif /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */

#if !defined(AUGMOD_BUILD)
# define AUGMOD_API AUGMOD_EXTERNC AUGMOD_IMPORT
#else /* AUGMOD_BUILD */
# define AUGMOD_API AUGMOD_EXTERNC AUGMOD_EXPORT
#endif /* AUGMOD_BUILD */

/* Also defined in augutil/var.h. */

#if !defined(AUG_VARTYPE)
# define AUG_VARTYPE
struct aug_vartype {
    int (*destroy_)(void*);
    const void* (*buf_)(void*, size_t*);
};
#endif /* !AUG_VARTYPE */

/* Also defined in augutil/var.h. */

#if !defined(AUG_VAR)
# define AUG_VAR
struct aug_var {
    const struct aug_vartype* type_;
    void* arg_;
};
#endif /* AUG_VAR */

#define augmod_vartype aug_vartype
#define augmod_var     aug_var

enum augmod_loglevel {
    AUGMOD_LOGCRIT,
    AUGMOD_LOGERROR,
    AUGMOD_LOGWARN,
    AUGMOD_LOGNOTICE,
    AUGMOD_LOGINFO,
    AUGMOD_LOGDEBUG
};

/**
   \defgroup TimerFlags Timer Flags
   \{
*/

/**
   Read timer.
*/

#define AUGMOD_TIMRD    0x01

/**
   Write timer.
*/

#define AUGMOD_TIMWR    0x02

/**
   Both read and write timer.
*/

#define AUGMOD_TIMRDWR (AUGMOD_TIMRD | AUGMOD_TIMWR)

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

#define AUGMOD_SHUTNOW  0x01

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

#define AUGMOD_OK      0

/**
   Failure.
*/

#define AUGMOD_ERROR (-1)

/**
   None, empty or null depending on context.
*/

#define AUGMOD_NONE  (-2)

/**
   /}
*/

#define AUGMOD_MAXNAME     63

typedef int augmod_id;

struct augmod_session {
    char name_[AUGMOD_MAXNAME + 1];
    void* user_;
};

/**
   Both sockets are timers are represented by objects.
*/

struct augmod_object {
    augmod_id id_;
    void* user_;
};

struct augmod_host {

    /**
       The following functions are thread-safe.
    */

    /**
       Write message to the runtime log.

       \param level The log level.

       \param format Printf-style specification.

       \param ... Arguments to format specification.

       \sa #augmod_loglevel, vwritelog_().
    */

    void (*writelog_)(int level, const char* format, ...);

    /**
       Write message to the runtime log.

       \param level The log level.

       \param format Printf-style specification.

       \param ... Arguments to format specification.

       \sa #augmod_loglevel, writelog_().
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

       \param type Type name associated with "var".

       \param user Optional user data.

       \sa dispatch_()
    */

    int (*post_)(const char* to, const char* type,
                 const struct augmod_var* user);

    /**
       The remaining functions are not thread-safe.
    */

    /**
       Dispatch event to peer session.

       \param to Target session name.

       \param type Type name associated with "user".

       \param user Optional user data.

       \param size Size of optional user data.

       \sa post_()
    */

    int (*dispatch_)(const char* to, const char* type, const void* user,
                     size_t size);

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

    const struct augmod_session* (*getsession_)(void);

    /**
       Shutdown the connection.

       \param cid Connection id.

       \param flags Use #AUGMOD_SHUTNOW to force immediate closure of the
       connection - do not wait for pending writes.
    */

    int (*shutdown_)(augmod_id cid, unsigned flags);

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

    int (*send_)(augmod_id cid, const void* buf, size_t len);

    /**
       Send data to peer.

       \param cid Connection id.

       \param user User data.
    */

    int (*sendv_)(augmod_id cid, const struct augmod_var* user);

    /**
       Set read/write timer.

       \param cid Connection id.

       \param ms Timeout value in milliseconds.

       \param flags \ref TimerFlags.
    */

    int (*setrwtimer_)(augmod_id cid, unsigned ms, unsigned flags);

    /**
       Reset read/write timer.

       \param cid Connection id.

       \param ms Timeout value in milliseconds.

       \param flags \ref TimerFlags.
    */

    int (*resetrwtimer_)(augmod_id cid, unsigned ms, unsigned flags);

    /**
       Cancel read/write timer.

       \param cid Connection id.

       \param flags \ref TimerFlags.
    */

    int (*cancelrwtimer_)(augmod_id cid, unsigned flags);


    /**
       Create new timer.

       \param ms Timeout value in milliseconds.

       \param user Optional user data.
    */

    int (*settimer_)(unsigned ms, const struct augmod_var* user);

    /**
       Reset timer.

       \param tid Timer id.

       \param ms Timeout value in milliseconds.
    */

    int (*resettimer_)(augmod_id tid, unsigned ms);

    /**
       Cancel timer.

       \param tid Timer id.
    */

    int (*canceltimer_)(augmod_id tid);

    /**
       Set ssl client.

       \param cid Connection id.

       \param ctx SSL context.
    */

    int (*setsslclient_)(augmod_id cid, const char* ctx);

    /**
       Set ssl server.

       \param cid Connection id.

       \param ctx SSL context.
    */

    int (*setsslserver_)(augmod_id cid, const char* ctx);
};

/**
   Module functions of type int should return either #AUGMOD_OK or
   #AUGMOD_ERROR, depending on the result.  For those functions associated
   with a connection, a failure will result in the connection being closed.
*/

struct augmod_proxy {

    /**
       Stop session.

       The current session can be retrieved using getsession_().  All
       resources associated with the session should be released in this
       handler.  stop_() will only be called for a session if start_()
       returned #AUGMOD_OK.
    */

    void (*stop_)(void);

    /**
       Start session.

       User-state associated with the session may be assigned to
       "session->user_".

       \return either #AUGMOD_OK or #AUGMOD_ERROR.
    */

    int (*start_)(struct augmod_session* session);

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

       \param user User data.

       \param size Size of user data.
    */

    void (*event_)(const char* from, const char* type, const void* user,
                   size_t size);

    /**
       Connection closure.

       \param sock The closed socket.
    */

    void (*closed_)(const struct augmod_object* sock);

    /**
       Teardown request.

       \param sock TODO
    */

    void (*teardown_)(const struct augmod_object* sock);

    /**
       Acceptance of socket connection.

       This function is called when a new connection is accepted on a listener
       socket.

       \param sock TODO

       \param addr TODO

       \param port TODO

       \return either #AUGMOD_OK or #AUGMOD_ERROR.
    */

    int (*accepted_)(struct augmod_object* sock, const char* addr,
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

    void (*connected_)(struct augmod_object* sock, const char* addr,
                       unsigned short port);

    /**
       Inbound data.

       \param sock The socket on which the data was received.

       \param buf Data buffer.  May not be null terminated.

       \param len Length of data buffer.
    */

    void (*data_)(const struct augmod_object* sock, const void* buf,
                  size_t len);

    /**
       Expiry of read timer.

       \param sock TODO

       \param ms The current timeout value.  The callee may modify "ms" to
       specify a new value; a value of zero will cancel the timer.
    */

    void (*rdexpire_)(const struct augmod_object* sock, unsigned* ms);

    /**
       Expiry of write timer.

       \param sock TODO

       \param ms The current timeout value.  The callee may modify "ms" to
       specify a new value; a value of zero will cancel the timer.
    */

    void (*wrexpire_)(const struct augmod_object* sock, unsigned* ms);

    /**
       Timer expiry.

       \param timer TODO

       \param ms The current timeout value.  The callee may modify "ms" to
       specify a new value; a value of zero will cancel the timer.
    */

    void (*expire_)(const struct augmod_object* timer, unsigned* ms);

    /**
       Authorisation of peer certificate.

       \param sock TODO

       \param subject TODO

       \param issuer TODO

       \return either #AUGMOD_OK or #AUGMOD_ERROR.
    */

    int (*authcert_)(const struct augmod_object* sock, const char* subject,
                     const char* issuer);
};

AUGMOD_EXTERNC const struct augmod_host*
augmod_gethost(void);

/**
   Syntactic sugar that allows host functions to be called with a
   function-like syntax.
*/

#define augmod_writelog      (augmod_gethost()->writelog_)
#define augmod_vwritelog     (augmod_gethost()->vwritelog_)
#define augmod_error         (augmod_gethost()->error_)
#define augmod_reconfall     (augmod_gethost()->reconfall_)
#define augmod_stopall       (augmod_gethost()->stopall_)
#define augmod_post          (augmod_gethost()->post_)
#define augmod_dispatch      (augmod_gethost()->dispatch_)
#define augmod_getenv        (augmod_gethost()->getenv_)
#define augmod_getsession    (augmod_gethost()->getsession_)
#define augmod_shutdown      (augmod_gethost()->shutdown_)
#define augmod_tcpconnect    (augmod_gethost()->tcpconnect_)
#define augmod_tcplisten     (augmod_gethost()->tcplisten_)
#define augmod_send          (augmod_gethost()->send_)
#define augmod_sendv         (augmod_gethost()->sendv_)
#define augmod_setrwtimer    (augmod_gethost()->setrwtimer_)
#define augmod_resetrwtimer  (augmod_gethost()->resetrwtimer_)
#define augmod_cancelrwtimer (augmod_gethost()->cancelrwtimer_)
#define augmod_settimer      (augmod_gethost()->settimer_)
#define augmod_resettimer    (augmod_gethost()->resettimer_)
#define augmod_canceltimer   (augmod_gethost()->canceltimer_)
#define augmod_setsslclient  (augmod_gethost()->setsslclient_)
#define augmod_setsslserver  (augmod_gethost()->setsslserver_)

/**
   This macro defines the module's entry points.  augmod_init() should return
   NULL on failure.
*/

#define AUGMOD_ENTRYPOINTS(init, term)                              \
    static const struct augmod_host* host_ = NULL;                  \
    AUGMOD_EXTERNC const struct augmod_host*                        \
    augmod_gethost(void)                                            \
    {                                                               \
        return host_;                                               \
    }                                                               \
    AUGMOD_API const struct augmod_proxy*                           \
    augmod_init(const char* name, const struct augmod_host* host)   \
    {                                                               \
        if (host_)                                                  \
            return NULL;                                            \
        host_ = host;                                               \
        return init(name);                                          \
    }                                                               \
    AUGMOD_API void                                                 \
    augmod_term(void)                                               \
    {                                                               \
        term();                                                     \
        host_ = NULL;                                               \
    }

typedef void (*augmod_termfn)(void);
typedef const struct augmod_proxy*
(*augmod_initfn)(const char*, const struct augmod_host*);

#endif /* AUGMOD_H */
