/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_H
#define AUGAS_H

#include <stdarg.h>    /* va_list */
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

struct augas_event {
    char type_[AUGAS_MAXNAME + 1];
    void* user_;
    size_t size_;
};

struct augas_object {
    const struct augas_serv* serv_;
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
       \sa stop_().
    */

    void (*reconf_)(void);

    /**
       \brief Stop the application server.
       \sa reconf_().
    */

    void (*stop_)(void);

    /**
       \brief Post an event to the event queue.
       \param sname TODO
       \param to TODO
       \param event TODO
       \param destr TODO
       \return TODO
       \sa TODO
    */

    int (*post_)(const char* sname, const char* to,
                 const struct augas_event* event, void (*destr)(void*));

    /**
     * The remaining functions are not thread-safe.
     */

    /**
       \brief Invoke peer service with event.
       \param sname TODO
       \param to TODO
       \param event TODO
       \return TODO
       \sa post_()
    */

    int (*invoke_)(const char* sname, const char* to,
                   const struct augas_event* event);

    /**
       \brief Read a configuration value.
       \param name TODO
       \return TODO
       \sa TODO

       If the specified value does not exist in the configuration file, an
       attempt will be made to read the value from the environment table.
    */

    const char* (*getenv_)(const char* name);

    /**
       \brief TODO
       \param sid TODO
       \return TODO
       \sa TODO
    */

    int (*shutdown_)(augas_id sid);

    /**
       \brief TODO
       \param sname Service name.
       \param host Ip address or host name.
       \param port Port or service name.
       \param user User data to be associated with the resulting connection.
       \return The connection id.
       \sa TODO

       This function will always return with the connection-id before the
       module is notified of connection establishment.
    */

    int (*tcpconnect_)(const char* sname, const char* host, const char* port,
                       void* user);

    /**
       \brief TODO
       \param sname TODO
       \param host TODO
       \param port TODO
       \param user TODO
       \return TODO
       \sa TODO
    */

    int (*tcplisten_)(const char* sname, const char* host, const char* port,
                      void* user);

    /**
       \brief TODO
       \param cid TODO
       \param buf TODO
       \param size TODO
       \return TODO
       \sa TODO

       Data may be written to a client connection that has not been fully
       established.  In which case, the data will be buffered for writing once
       the connection has been established.
    */

    int (*send_)(augas_id cid, const char* buf, size_t size);

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
       \param sname TODO
       \param ms TODO
       \param user TODO
       \param destr TODO
       \return TODO
       \sa TODO
    */

    int (*settimer_)(const char* sname, unsigned ms, void* user,
                     void (*destr)(void*));

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
       \brief Service termination.
       \param serv TODO
       \return TODO
       \sa TODO
    */

    void (*term_)(const struct augas_serv* serv);

    /**
       \brief Service initialisation.
       \param serv TODO
       \return TODO
       \sa TODO
    */

    int (*init_)(struct augas_serv* serv);

    /**
       \brief Re-configure request.
       \param serv TODO
       \return TODO
       \sa TODO
    */

    int (*reconf_)(const struct augas_serv* serv);

    /**
       \brief Custom event notification.
       \param serv TODO
       \param from TODO
       \param event TODO
       \return TODO
       \sa TODO
    */

    int (*event_)(const struct augas_serv* serv, const char* from,
                  const struct augas_event* event);

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

    int (*teardown_)(const struct augas_object* sock);

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

    int (*connected_)(struct augas_object* sock, const char* addr,
                      unsigned short port);

    /**
       \brief Inbound data.
       \param sock TODO
       \param buf TODO
       \param size TODO
       \return TODO
       \sa TODO
    */

    int (*data_)(const struct augas_object* sock, const char* buf,
                 size_t size);

    /**
       \brief Expiry of read timer.
       \param sock TODO
       \param ms TODO
       \return TODO
       \sa TODO
    */

    int (*rdexpire_)(const struct augas_object* sock, unsigned* ms);

    /**
       \brief Expiry of write timer.
       \param sock TODO
       \param ms TODO
       \return TODO
       \sa TODO
    */

    int (*wrexpire_)(const struct augas_object* sock, unsigned* ms);

    /**
       \brief Timer expiry.
       \param timer TODO
       \param ms TODO
       \return TODO
       \sa TODO
    */

    int (*expire_)(const struct augas_object* timer, unsigned* ms);
};

/**
   augas_load() should return NULL on failure.
*/

#define AUGAS_MODULE(load, unload)                                    \
    AUGAS_API void                                                    \
    augas_unload(void)                                                \
    {                                                                 \
        (*unload)();                                                  \
    }                                                                 \
    AUGAS_API const struct augas_module*                              \
    augas_load(const char* name, const struct augas_host* host)       \
    {                                                                 \
        return (*load)(name, host);                                   \
    }

typedef void (*augas_unloadfn)(void);
typedef const struct augas_module* (*augas_loadfn)(const char*,
                                                   const struct augas_host*);

#endif /* AUGAS_H */
