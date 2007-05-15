/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_H
#define AUGAS_H

#include <stdarg.h>    /* va_list */
#include <stdlib.h>    /* NULL */
#include <sys/types.h> /* size_t */

#if !defined(__cplusplus)
# define AUGAS_EXTERN extern
#else /* __cplusplus */
# define AUGAS_EXTERN extern "C"
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUGAS_EXPORT AUGAS_EXTERN
# define AUGAS_IMPORT AUGAS_EXTERN
#else /* _WIN32 */
# define AUGAS_EXPORT AUGAS_EXTERN __declspec(dllexport)
# define AUGAS_IMPORT AUGAS_EXTERN __declspec(dllimport)
#endif /* _WIN32 */

#if !defined(AUGAS_BUILD)
# define AUGAS_API AUGAS_EXPORT
#else /* AUGAS_BUILD */
# define AUGAS_API AUGAS_IMPORT
#endif /* AUGAS_BUILD */

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

#define augas_vartype aug_vartype
#define augas_var     aug_var

enum augas_loglevel {
    AUGAS_LOGCRIT,
    AUGAS_LOGERROR,
    AUGAS_LOGWARN,
    AUGAS_LOGNOTICE,
    AUGAS_LOGINFO,
    AUGAS_LOGDEBUG
};

/**
   \defgroup TimerFlags Timer Flags
   \{
*/

/**
   Read timer.
 */

#define AUGAS_TIMRD    0x01

/**
   Write timer.
 */

#define AUGAS_TIMWR    0x02

/**
   Both read and write timer.
 */

#define AUGAS_TIMBOTH (AUGAS_TIMRD | AUGAS_TIMWR)

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

#define AUGAS_OK        0

/**
   Failure.
 */

#define AUGAS_ERROR   (-1)

/**
   None, empty or null depending on context.
 */

#define AUGAS_NONE    (-2)

/**
   /}
 */

#define AUGAS_MAXNAME  63

typedef int augas_id;

struct augas_serv {
    char name_[AUGAS_MAXNAME + 1];
    void* user_;
};

struct augas_object {
    augas_id id_;
    void* user_;
};

struct augas_host {

    /**
       The following functions are thread-safe.
     */

    /**
       Write message to the application server's log.

       \param level The log level.
       \param format Printf-style specification.
       \param ... Arguments to format specification.
       \sa #augas_loglevel, vwritelog_().
    */

    void (*writelog_)(int level, const char* format, ...);

    /**
       Write message to the application server's log.

       \param level The log level.
       \param format Printf-style specification.
       \param ... Arguments to format specification.
       \sa #augas_loglevel, writelog_().
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
       Stop the application server.

       \sa reconfall_().
    */

    int (*stopall_)(void);

    /**
       Post an event to the event queue.

       \param to Target service name.
       \param type Type name associated with "var".
       \param var User data.
       \sa dispatch_()
    */

    int (*post_)(const char* to, const char* type,
                 const struct augas_var* var);

    /**
       The remaining functions are not thread-safe.
    */

    /**
       Dispatch event to peer service.

       \param to Target service name.
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
       Get the active service.
    */

    const struct augas_serv* (*getserv_)(void);

    /**
       Shutdown the connection.

       \param cid Connection id.
    */

    int (*shutdown_)(augas_id cid);

    /**
       Establish tcp connection.

       This function will always return with the connection-id before the
       module is notified of connection establishment.

       \param host Ip address or host name.
       \param port Port or service name.
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

    int (*send_)(augas_id cid, const void* buf, size_t len);

    /**
       Send data to peer.

       \param cid Connection id.
       \param var User data.
    */

    int (*sendv_)(augas_id cid, const struct augas_var* var);

    /**
       Set read/write timer.

       \param cid Connection id.
       \param ms Timeout value in milliseconds.
       \param flags \ref TimerFlags.
    */

    int (*setrwtimer_)(augas_id cid, unsigned ms, unsigned flags);

    /**
       Reset read/write timer.

       \param cid Connection id.
       \param ms Timeout value in milliseconds.
       \param flags \ref TimerFlags.
    */

    int (*resetrwtimer_)(augas_id cid, unsigned ms, unsigned flags);

    /**
       Cancel read/write timer.

       \param cid Connection id.
       \param flags \ref TimerFlags.
    */

    int (*cancelrwtimer_)(augas_id cid, unsigned flags);


    /**
       Create new timer.

       \param ms Timeout value in milliseconds.
       \param var User data.
    */

    int (*settimer_)(unsigned ms, const struct augas_var* var);

    /**
       Reset timer.

       \param tid Timer id.
       \param ms Timeout value in milliseconds.
    */

    int (*resettimer_)(augas_id tid, unsigned ms);

    /**
       Cancel timer.

       \param tid Timer id.
    */

    int (*canceltimer_)(augas_id tid);

    /**
       Set ssl client.

       \param cid Connection id.
       \param ctx SSL context.
    */

    int (*setsslclient_)(augas_id cid, const char* ctx);

    /**
       Set ssl server.

       \param cid Connection id.
       \param ctx SSL context.
    */

    int (*setsslserver_)(augas_id cid, const char* ctx);
};

/**
   Module functions should return either #AUGAS_OK or #AUGAS_ERROR.  For those
   functions associated with a connection, a failure will result in the
   connection being closed.
*/

struct augas_module {

    /**
       Stop service.
    */

    void (*stop_)(void);

    /**
       Start service.
    */

    int (*start_)(struct augas_serv* serv);

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

    void (*closed_)(const struct augas_object* sock);

    /**
       Teardown request.

       \param sock TODO
    */

    void (*teardown_)(const struct augas_object* sock);

    /**
       Acceptance of socket connection.

       \param sock TODO
       \param addr TODO
       \param port TODO
    */

    int (*accept_)(struct augas_object* sock, const char* addr,
                   unsigned short port);

    /**
       Completion of client connection handshake.

       \param sock TODO
       \param addr TODO
       \param port TODO
    */

    void (*connected_)(struct augas_object* sock, const char* addr,
                       unsigned short port);

    /**
       Inbound data.

       \param sock TODO
       \param buf TODO
       \param len TODO
    */

    void (*data_)(const struct augas_object* sock, const void* buf,
                  size_t len);

    /**
       Expiry of read timer.

       \param sock TODO
       \param ms TODO
    */

    void (*rdexpire_)(const struct augas_object* sock, unsigned* ms);

    /**
       Expiry of write timer.

       \param sock TODO
       \param ms TODO
    */

    void (*wrexpire_)(const struct augas_object* sock, unsigned* ms);

    /**
       Timer expiry.

       \param timer TODO
       \param ms TODO
    */

    void (*expire_)(const struct augas_object* timer, unsigned* ms);

    /**
       Authorisation of peer certificate.

       \param sock TODO
       \param subject TODO
       \param issuer TODO
    */

    int (*authcert_)(const struct augas_object* sock, const char* subject,
                     const char* issuer);
};

AUGAS_EXTERN const struct augas_host*
augas_gethost(void);

#define augas_writelog      (augas_gethost()->writelog_)
#define augas_vwritelog     (augas_gethost()->vwritelog_)
#define augas_error         (augas_gethost()->error_)
#define augas_reconfall     (augas_gethost()->reconfall_)
#define augas_stopall       (augas_gethost()->stopall_)
#define augas_post          (augas_gethost()->post_)
#define augas_dispatch      (augas_gethost()->dispatch_)
#define augas_getenv        (augas_gethost()->getenv_)
#define augas_getserv       (augas_gethost()->getserv_)
#define augas_shutdown      (augas_gethost()->shutdown_)
#define augas_tcpconnect    (augas_gethost()->tcpconnect_)
#define augas_tcplisten     (augas_gethost()->tcplisten_)
#define augas_send          (augas_gethost()->send_)
#define augas_sendv         (augas_gethost()->sendv_)
#define augas_setrwtimer    (augas_gethost()->setrwtimer_)
#define augas_resetrwtimer  (augas_gethost()->resetrwtimer_)
#define augas_cancelrwtimer (augas_gethost()->cancelrwtimer_)
#define augas_settimer      (augas_gethost()->settimer_)
#define augas_resettimer    (augas_gethost()->resettimer_)
#define augas_canceltimer   (augas_gethost()->canceltimer_)
#define augas_setsslclient  (augas_gethost()->setsslclient_)
#define augas_setsslserver  (augas_gethost()->setsslserver_)

/**
   augas_init() should return NULL on failure.
*/

#define AUGAS_MODULE(init, term)                                      \
    static const struct augas_host* host_ = NULL;                     \
    AUGAS_EXTERN const struct augas_host*                             \
    augas_gethost(void)                                               \
    {                                                                 \
        return host_;                                                 \
    }                                                                 \
    AUGAS_API const struct augas_module*                              \
    augas_init(const char* name, const struct augas_host* host)       \
    {                                                                 \
        if (host_)                                                    \
            return NULL;                                              \
        host_ = host;                                                 \
        return (*init)(name);                                         \
    }                                                                 \
    AUGAS_API void                                                    \
    augas_term(void)                                                  \
    {                                                                 \
        (*term)();                                                    \
        host_ = NULL;                                                 \
    }

typedef void (*augas_termfn)(void);
typedef const struct augas_module* (*augas_initfn)(const char*,
                                                   const struct augas_host*);

#endif /* AUGAS_H */
