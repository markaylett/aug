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

#define AUGAS_TIMRD    0x01
#define AUGAS_TIMWR    0x02
#define AUGAS_TIMBOTH (AUGAS_TIMRD | AUGAS_TIMWR)

#define AUGAS_OK        0
#define AUGAS_ERROR   (-1)
#define AUGAS_NONE    (-2)

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
     * The following functions are thread-safe.
     */

    /**
       \brief Write message to the application server's log.
       \param level TODO
       \param format TODO
       \param ... TODO
       \sa vwritelog_().
    */

    void (*writelog_)(int level, const char* format, ...);

    /**
       \brief Write message to the application server's log.
       \param level TODO
       \param format TODO
       \param args TODO
       \return TODO
       \sa writelog_().
    */

    void (*vwritelog_)(int level, const char* format, va_list args);

    /**
       \brief Get a description of the last host error.
       \return The error description.

       If the return values of any host function indicates failure, this
       function can be used to obtain a description of the error.
    */

    const char* (*error_)(void);

    /**
       \brief Re-configure the host and all loaded modules.
       \return TODO
       \sa stopall_().
    */

    int (*reconf_)(void);

    /**
       \brief Stop the application server.
       \return TODO
       \sa reconf_().
    */

    int (*stopall_)(void);

    /**
       \brief Post an event to the event queue.
       \param to TODO
       \param type TODO
       \param var TODO
       \return TODO
       \sa TODO
    */

    int (*post_)(const char* to, const char* type,
                 const struct augas_var* var);

    /**
     * The remaining functions are not thread-safe.
     */

    /**
       \brief Dispatch event to peer service.
       \param to TODO
       \param type TODO
       \param user TODO
       \param size TODO
       \return TODO
       \sa post_()
    */

    int (*dispatch_)(const char* to, const char* type, const void* user,
                     size_t size);

    /**
       \brief Read a configuration value.
       \param name TODO
       \param def TODO
       \return TODO
       \sa TODO

       If the specified value does not exist in the configuration file, an
       attempt will be made to read the value from the environment table.
    */

    const char* (*getenv_)(const char* name, const char* def);

    /**
       \brief Get the active service.
       \return TODO
       \sa TODO
    */

    const struct augas_serv* (*getserv_)(void);

    /**
       \brief TODO
       \param sid TODO
       \return TODO
       \sa TODO
    */

    int (*shutdown_)(augas_id sid);

    /**
       \brief TODO
       \param host Ip address or host name.
       \param port Port or service name.
       \param user User data to be associated with the resulting connection.
       \return The connection id.
       \sa TODO

       This function will always return with the connection-id before the
       module is notified of connection establishment.
    */

    int (*tcpconnect_)(const char* host, const char* port, void* user);

    /**
       \brief TODO
       \param host TODO
       \param port TODO
       \param user TODO
       \return TODO
       \sa TODO
    */

    int (*tcplisten_)(const char* host, const char* port, void* user);

    /**
       \brief TODO
       \param cid TODO
       \param buf TODO
       \param len TODO
       \return TODO
       \sa TODO

       Data may be written to a client connection that has not been fully
       established.  In which case, the data will be buffered for writing once
       the connection has been established.
    */

    int (*send_)(augas_id cid, const void* buf, size_t len);

    /**
       \brief TODO
       \param cid TODO
       \param var TODO
       \return TODO
       \sa TODO
    */

    int (*sendv_)(augas_id cid, const struct augas_var* var);

    /**
       \brief TODO
       \param cid TODO
       \param ms TODO
       \param flags TODO
       \return TODO
       \sa TODO
    */

    int (*setrwtimer_)(augas_id cid, unsigned ms, unsigned flags);

    /**
       \brief TODO
       \param cid TODO
       \param ms TODO
       \param flags TODO
       \return TODO
       \sa TODO
    */

    int (*resetrwtimer_)(augas_id cid, unsigned ms, unsigned flags);

    /**
       \brief TODO
       \param cid TODO
       \param flags TODO
       \return TODO
       \sa TODO
    */

    int (*cancelrwtimer_)(augas_id cid, unsigned flags);


    /**
       \brief TODO
       \param ms TODO
       \param var TODO
       \return TODO
       \sa TODO
    */

    int (*settimer_)(unsigned ms, const struct augas_var* var);

    /**
       \brief TODO
       \param tid TODO
       \param ms TODO
       \return TODO
       \sa TODO
    */

    int (*resettimer_)(augas_id tid, unsigned ms);

    /**
       \brief TODO
       \param tid TODO
       \return TODO
       \sa TODO
    */

    int (*canceltimer_)(augas_id tid);
};

/**
   Module functions should return either #AUGAS_OK or #AUGAS_ERROR.  For those
   functions associated with a connection, a failure will result in the
   connection being closed.
*/

struct augas_module {

    /**
       \brief Stop service.
       \return TODO
       \sa TODO
    */

    void (*stop_)(void);

    /**
       \brief Start service.
       \return TODO
       \sa TODO
    */

    int (*start_)(struct augas_serv* serv);

    /**
       \brief Re-configure request.
       \return TODO
       \sa TODO
    */

    void (*reconf_)(void);

    /**
       \brief Custom event notification.
       \param from TODO
       \param type TODO
       \param user TODO
       \param size TODO
       \return TODO
       \sa TODO
    */

    void (*event_)(const char* from, const char* type, const void* user,
                   size_t size);

    /**
       \brief Connection closure.
       \param sock TODO
       \return TODO
       \sa TODO
    */

    void (*closed_)(const struct augas_object* sock);

    /**
       \brief Teardown request.
       \param sock TODO
       \return TODO
       \sa TODO
    */

    void (*teardown_)(const struct augas_object* sock);

    /**
       \brief Acceptance of socket connection.
       \param sock TODO
       \param addr TODO
       \param port TODO
       \return TODO
       \sa TODO
    */

    int (*accept_)(struct augas_object* sock, const char* addr,
                   unsigned short port);

    /**
       \brief Completion of client connection handshake.
       \param sock TODO
       \param addr TODO
       \param port TODO
       \return TODO
       \sa TODO
    */

    void (*connected_)(struct augas_object* sock, const char* addr,
                       unsigned short port);

    /**
       \brief Inbound data.
       \param sock TODO
       \param buf TODO
       \param len TODO
       \return TODO
       \sa TODO
    */

    void (*data_)(const struct augas_object* sock, const void* buf,
                  size_t len);

    /**
       \brief Expiry of read timer.
       \param sock TODO
       \param ms TODO
       \return TODO
       \sa TODO
    */

    void (*rdexpire_)(const struct augas_object* sock, unsigned* ms);

    /**
       \brief Expiry of write timer.
       \param sock TODO
       \param ms TODO
       \return TODO
       \sa TODO
    */

    void (*wrexpire_)(const struct augas_object* sock, unsigned* ms);

    /**
       \brief Timer expiry.
       \param timer TODO
       \param ms TODO
       \return TODO
       \sa TODO
    */

    void (*expire_)(const struct augas_object* timer, unsigned* ms);
};

AUGAS_EXTERN const struct augas_host*
augas_gethost(void);

#define augas_writelog      (augas_gethost()->writelog_)
#define augas_vwritelog     (augas_gethost()->vwritelog_)
#define augas_error         (augas_gethost()->error_)
#define augas_reconf        (augas_gethost()->reconf_)
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
