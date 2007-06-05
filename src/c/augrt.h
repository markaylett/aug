/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRT_H
#define AUGRT_H

#include <stdarg.h>    /* va_list */
#include <stdlib.h>    /* NULL */
#include <sys/types.h> /* size_t */

#if !defined(__cplusplus)
# define AUGRT_EXTERN extern
#else /* __cplusplus */
# define AUGRT_EXTERN extern "C"
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUGRT_EXPORT AUGRT_EXTERN
# define AUGRT_IMPORT AUGRT_EXTERN
#else /* _WIN32 */
# define AUGRT_EXPORT AUGRT_EXTERN __declspec(dllexport)
# define AUGRT_IMPORT AUGRT_EXTERN __declspec(dllimport)
#endif /* _WIN32 */

#if !defined(AUGRT_BUILD)
# define AUGRT_API AUGRT_EXPORT
#else /* AUGRT_BUILD */
# define AUGRT_API AUGRT_IMPORT
#endif /* AUGRT_BUILD */

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

#define augrt_vartype aug_vartype
#define augrt_var     aug_var

enum augrt_loglevel {
    AUGRT_LOGCRIT,
    AUGRT_LOGERROR,
    AUGRT_LOGWARN,
    AUGRT_LOGNOTICE,
    AUGRT_LOGINFO,
    AUGRT_LOGDEBUG
};

/**
   \defgroup TimerFlags Timer Flags
   \{
*/

/**
   Read timer.
 */

#define AUGRT_TIMRD    0x01

/**
   Write timer.
 */

#define AUGRT_TIMWR    0x02

/**
   Both read and write timer.
 */

#define AUGRT_TIMBOTH (AUGRT_TIMRD | AUGRT_TIMWR)

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

#define AUGRT_OK      0

/**
   Failure.
 */

#define AUGRT_ERROR (-1)

/**
   None, empty or null depending on context.
 */

#define AUGRT_NONE  (-2)

/**
   /}
 */

#define AUGRT_MCAST        0
#define AUGRT_MAXNAME     63
#define AUGRT_PACKETSIZE 512

typedef int augrt_id;

struct augrt_session {
    char name_[AUGRT_MAXNAME + 1];
    void* user_;
};

struct augrt_object {
    augrt_id id_;
    void* user_;
};

struct augrt_host {

    /**
       The following functions are thread-safe.
     */

    /**
       Write message to the runtime log.

       \param level The log level.
       \param format Printf-style specification.
       \param ... Arguments to format specification.
       \sa #augrt_loglevel, vwritelog_().
    */

    void (*writelog_)(int level, const char* format, ...);

    /**
       Write message to the runtime log.

       \param level The log level.
       \param format Printf-style specification.
       \param ... Arguments to format specification.
       \sa #augrt_loglevel, writelog_().
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
       \param var User data.
       \sa dispatch_()
    */

    int (*post_)(const char* to, const char* type,
                 const struct augrt_var* var);

    /**
       The remaining functions are not thread-safe.
    */

    /**
       Dispatch event to peer session.

       \param to Target session name.
       \param type Type name associated with "user".
       \param user User data.
       \param size Size of user data.
       \sa post_()
    */

    int (*dispatch_)(const char* to, const char* type, const void* user,
                     size_t size);

    /**
       Read a configuration value.

       If the specified value does not exist in the configuration file, an
       attempt will be made to read the value from the environment table.

       \param name Name or key of desired value.
       \param def The value to return by default.
       \return The default value if no value exists for name.
    */

    const char* (*getenv_)(const char* name, const char* def);

    /**
       Get the active session.
    */

    const struct augrt_session* (*getsession_)(void);

    /**
       Shutdown the connection.

       \param cid Connection id.
    */

    int (*shutdown_)(augrt_id cid);

    /**
       Establish tcp connection.

       This function will always return with the connection-id before the
       module is notified of connection establishment.

       \param host Ip address or host name.
       \param port Port or session name.
       \param user User data to be associated with the resulting connection.
       \return The connection id.
    */

    int (*tcpconnect_)(const char* host, const char* port, void* user);

    /**
       Bind tcp listener socket.

       \param host Host to be bound.
       \param port Port to be bound.
       \param user User data.
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

    int (*send_)(augrt_id cid, const void* buf, size_t len);

    /**
       Send data to peer.

       \param cid Connection id.
       \param var User data.
    */

    int (*sendv_)(augrt_id cid, const struct augrt_var* var);

    /**
       Set read/write timer.

       \param cid Connection id.
       \param ms Timeout value in milliseconds.
       \param flags \ref TimerFlags.
    */

    int (*setrwtimer_)(augrt_id cid, unsigned ms, unsigned flags);

    /**
       Reset read/write timer.

       \param cid Connection id.
       \param ms Timeout value in milliseconds.
       \param flags \ref TimerFlags.
    */

    int (*resetrwtimer_)(augrt_id cid, unsigned ms, unsigned flags);

    /**
       Cancel read/write timer.

       \param cid Connection id.
       \param flags \ref TimerFlags.
    */

    int (*cancelrwtimer_)(augrt_id cid, unsigned flags);


    /**
       Create new timer.

       \param ms Timeout value in milliseconds.
       \param var User data.
    */

    int (*settimer_)(unsigned ms, const struct augrt_var* var);

    /**
       Reset timer.

       \param tid Timer id.
       \param ms Timeout value in milliseconds.
    */

    int (*resettimer_)(augrt_id tid, unsigned ms);

    /**
       Cancel timer.

       \param tid Timer id.
    */

    int (*canceltimer_)(augrt_id tid);

    /**
       Set ssl client.

       \param cid Connection id.
       \param ctx SSL context.
    */

    int (*setsslclient_)(augrt_id cid, const char* ctx);

    /**
       Set ssl server.

       \param cid Connection id.
       \param ctx SSL context.
    */

    int (*setsslserver_)(augrt_id cid, const char* ctx);
};

/**
   Module functions should return either #AUGRT_OK or #AUGRT_ERROR.  For those
   functions associated with a connection, a failure will result in the
   connection being closed.
*/

struct augrt_module {

    /**
       Stop session.
    */

    void (*stop_)(void);

    /**
       Start session.
    */

    int (*start_)(struct augrt_session* session);

    /**
       Re-configure request.
    */

    void (*reconf_)(void);

    /**
       Custom event notification.

       \param from TODO
       \param type TODO
       \param user TODO
       \param size TODO
    */

    void (*event_)(const char* from, const char* type, const void* user,
                   size_t size);

    /**
       Connection closure.

       \param sock TODO
    */

    void (*closed_)(const struct augrt_object* sock);

    /**
       Teardown request.

       \param sock TODO
    */

    void (*teardown_)(const struct augrt_object* sock);

    /**
       Acceptance of socket connection.

       \param sock TODO
       \param addr TODO
       \param port TODO
    */

    int (*accepted_)(struct augrt_object* sock, const char* addr,
                     unsigned short port);

    /**
       Completion of client connection handshake.

       \param sock TODO
       \param addr TODO
       \param port TODO
    */

    void (*connected_)(struct augrt_object* sock, const char* addr,
                       unsigned short port);

    /**
       Inbound data.

       \param sock TODO
       \param buf TODO
       \param len TODO
    */

    void (*data_)(const struct augrt_object* sock, const void* buf,
                  size_t len);

    /**
       Expiry of read timer.

       \param sock TODO
       \param ms TODO
    */

    void (*rdexpire_)(const struct augrt_object* sock, unsigned* ms);

    /**
       Expiry of write timer.

       \param sock TODO
       \param ms TODO
    */

    void (*wrexpire_)(const struct augrt_object* sock, unsigned* ms);

    /**
       Timer expiry.

       \param timer TODO
       \param ms TODO
    */

    void (*expire_)(const struct augrt_object* timer, unsigned* ms);

    /**
       Authorisation of peer certificate.

       \param sock TODO
       \param subject TODO
       \param issuer TODO
    */

    int (*authcert_)(const struct augrt_object* sock, const char* subject,
                     const char* issuer);
};

AUGRT_EXTERN const struct augrt_host*
augrt_gethost(void);

#define augrt_writelog      (augrt_gethost()->writelog_)
#define augrt_vwritelog     (augrt_gethost()->vwritelog_)
#define augrt_error         (augrt_gethost()->error_)
#define augrt_reconfall     (augrt_gethost()->reconfall_)
#define augrt_stopall       (augrt_gethost()->stopall_)
#define augrt_post          (augrt_gethost()->post_)
#define augrt_dispatch      (augrt_gethost()->dispatch_)
#define augrt_getenv        (augrt_gethost()->getenv_)
#define augrt_getsession    (augrt_gethost()->getsession_)
#define augrt_shutdown      (augrt_gethost()->shutdown_)
#define augrt_tcpconnect    (augrt_gethost()->tcpconnect_)
#define augrt_tcplisten     (augrt_gethost()->tcplisten_)
#define augrt_send          (augrt_gethost()->send_)
#define augrt_sendv         (augrt_gethost()->sendv_)
#define augrt_setrwtimer    (augrt_gethost()->setrwtimer_)
#define augrt_resetrwtimer  (augrt_gethost()->resetrwtimer_)
#define augrt_cancelrwtimer (augrt_gethost()->cancelrwtimer_)
#define augrt_settimer      (augrt_gethost()->settimer_)
#define augrt_resettimer    (augrt_gethost()->resettimer_)
#define augrt_canceltimer   (augrt_gethost()->canceltimer_)
#define augrt_setsslclient  (augrt_gethost()->setsslclient_)
#define augrt_setsslserver  (augrt_gethost()->setsslserver_)

/**
   augrt_init() should return NULL on failure.
*/

#define AUGRT_MODULE(init, term)                                      \
    static const struct augrt_host* host_ = NULL;                     \
    AUGRT_EXTERN const struct augrt_host*                             \
    augrt_gethost(void)                                               \
    {                                                                 \
        return host_;                                                 \
    }                                                                 \
    AUGRT_API const struct augrt_module*                              \
    augrt_init(const char* name, const struct augrt_host* host)       \
    {                                                                 \
        if (host_)                                                    \
            return NULL;                                              \
        host_ = host;                                                 \
        return (*init)(name);                                         \
    }                                                                 \
    AUGRT_API void                                                    \
    augrt_term(void)                                                  \
    {                                                                 \
        (*term)();                                                    \
        host_ = NULL;                                                 \
    }

typedef void (*augrt_termfn)(void);
typedef const struct augrt_module* (*augrt_initfn)(const char*,
                                                   const struct augrt_host*);

#endif /* AUGRT_H */
