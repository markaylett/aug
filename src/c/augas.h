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

#define AUGAS_TIMRD     0x01
#define AUGAS_TIMWR     0x02
#define AUGAS_TIMBOTH  (AUGAS_TIMRD | AUGAS_TIMWR)

#define AUGAS_OK         0
#define AUGAS_ERROR    (-1)
#define AUGAS_NONE     (-2)

#define AUGAS_MAXNAME   63

typedef int augas_id;

struct augas_sess {
    char name_[AUGAS_MAXNAME + 1];
    void* user_;
};

struct augas_sock {
    const struct augas_sess* sess_;
    augas_id id_;
    void* user_;
};

struct augas_host {

    /**
     * The following functions are thread-safe.
     */

    /**
       \brief Get a description of the last error.
       \return error description

       If the return values of any host function indicates failure, this
       function can be used to obtain a description of the error.
    */

    const char* (*error_)(void);

    /**
       \brief Re-configure the server and all loaded modules.
       \sa TODO
    */

    void (*reconf_)(void);

    /**
       \brief Stop the server.
    */

    void (*stop_)(void);

    /**
       \brief Write message to server's log.
    */

    void (*writelog_)(int level, const char* format, ...);

    /**
       \brief Write message to server's log.
    */

    void (*vwritelog_)(int level, const char* format, va_list args);

    /**
       \brief Post an event to the event queue.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*post_)(const char* sname, int type, void* user,
                 void (*free)(void*));

    /**
     * The remaining functions are not thread-safe.
     */

    /**
       \brief Delegate event to a different session.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*delegate_)(const char* sname, int type, void* user);

    /**
       \brief Read a configuration value.
       \param TODO
       \return TODO
       \sa TODO

       If the specified value does not exist in the configuration file, an
       attempt will be made to read the value from the environment table.
    */

    const char* (*getenv_)(const char* name);

    /**
       \brief TODO
       \param sname Session name.
       \param host Ip address or host name.
       \param serv Port or service name.
       \param user User data to be associated with the resulting connection.
       \return The connection id.
       \sa TODO
    */

    int (*tcpconnect_)(const char* sname, const char* host, const char* serv,
                       void* user);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*tcplisten_)(const char* sname, const char* host, const char* serv,
                      void* user);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*settimer_)(const char* sname, augas_id tid, unsigned ms, void* user,
                     void (*free_)(void*));

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*resettimer_)(const char* sname, augas_id tid, unsigned ms);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*canceltimer_)(const char* sname, augas_id tid);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*shutdown_)(augas_id sid);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*send_)(const char* sname, augas_id cid, const char* buf,
                 size_t size);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*setrwtimer_)(augas_id cid, unsigned ms, unsigned flags);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*resetrwtimer_)(augas_id cid, unsigned ms, unsigned flags);

    /**
       \brief TODO
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*cancelrwtimer_)(augas_id cid, unsigned flags);
};

/**
   Module functions should return either #AUGAS_OK or #AUGAS_ERROR.  For those
   functions associated with a connection, a failure will result in the
   connection being closed.
*/

struct augas_module {

    /**
       \brief Session termination.
       \param TODO
       \return TODO
       \sa TODO
    */

    void (*term_)(const struct augas_sess* sess);

    /**
       \brief Session initialisation.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*init_)(struct augas_sess* sess);

    /**
       \brief Custom event notification.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*event_)(const struct augas_sess* sess, int type, void* user);

    /**
       \brief Timer expiry.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*expire_)(const struct augas_sess* sess, augas_id cid, void* user,
                   unsigned* ms);

    /**
       \brief Re-configure request.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*reconf_)(const struct augas_sess* sess);


    /**
       \brief Connection closure.
       \param TODO
       \return TODO
       \sa TODO
    */

    void (*closed_)(const struct augas_sock* sock);

    /**
       \brief Acceptance of server connection.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*accept_)(struct augas_sock* sock, const char* addr,
                   unsigned short port);

    /**
       \brief Completion of client connection handshake.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*connected_)(struct augas_sock* sock, const char* addr,
                      unsigned short port);

    /**
       \brief Inbound data.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*data_)(const struct augas_sock* sock, const char* buf, size_t size);

    /**
       \brief Expiry of read timer.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*rdexpire_)(const struct augas_sock* sock, unsigned* ms);

    /**
       \brief Expiry of write timer.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*wrexpire_)(const struct augas_sock* sock, unsigned* ms);

    /**
       \brief Teardown request.
       \param TODO
       \return TODO
       \sa TODO
    */

    int (*teardown_)(const struct augas_sock* sock);
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
