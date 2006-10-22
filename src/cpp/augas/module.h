/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MODULE_H
#define AUGAS_MODULE_H

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

#if defined(DLL_EXPORT)
# define AUGAS_SHARED
#endif /* DLL_EXPORT */

#if !defined(AUGAS_SHARED)
# define AUGAS_API AUGAS_EXTERN
#else /* AUGAS_SHARED */
# if !defined(AUGAS_BUILD)
#  define AUGAS_API AUGAS_IMPORT
# else /* AUGAS_BUILD */
#  define AUGAS_API AUGAS_EXPORT
# endif /* AUGAS_BUILD */
#endif /* AUGAS_SHARED */

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

#define AUGAS_SESSELF   0x01
#define AUGAS_SESOTHER  0x02
#define AUGAS_SESALL   (AUGAS_SESSELF | AUGAS_SESOTHER)

#define AUGAS_SUCCESS   0
#define AUGAS_FAILURE (-1)

typedef unsigned augas_sid;

struct augas_session {
    augas_sid sid_;
    void* user_;
};

struct augas_service {
    const char* (*error_)(void);
    const char* (*getenv_)(const char* name);
    void (*writelog_)(int level, const char* format, ...);
    void (*vwritelog_)(int level, const char* format, va_list args);
    int (*post_)(int type, void* arg);
    int (*settimer_)(int id, unsigned ms, void* arg);
    int (*resettimer_)(int id, unsigned ms);
    int (*canceltimer_)(int id);
    int (*shutdown_)(augas_sid id);
    int (*send_)(augas_sid id, const char* buf, size_t size, unsigned flags);
    int (*setrwtimer_)(augas_sid id, unsigned ms, unsigned flags);
    int (*resetrwtimer_)(augas_sid id, unsigned ms, unsigned flags);
    int (*cancelrwtimer_)(augas_sid id, unsigned flags);
};

/**
   Module functions should return either #AUGAS_SUCCESS or #AUGAS_FAILURE.
*/

struct augas_module {
    void (*close_)(const struct augas_session* s);
    int (*open_)(struct augas_session* s, const char* serv);
    int (*data_)(const struct augas_session* s, const char* buf, size_t size);
    int (*rdexpire_)(const struct augas_session* s, unsigned* ms);
    int (*wrexpire_)(const struct augas_session* s, unsigned* ms);
    int (*stop_)(const struct augas_session* s);
    int (*event_)(int type, void* arg);
    int (*expire_)(void* arg, unsigned id, unsigned* ms);
    int (*reconf_)(void);
};

/**
   augas_load() should return NULL on failure.
*/

#define AUGAS_MODULE(load, unload) \
AUGAS_API const struct augas_module* \
augas_load(const struct augas_service* service) \
{ \
    return (*load)(service); \
} \
AUGAS_API void \
augas_unload(void) \
{ \
    (*unload)(); \
}

typedef const struct augas_module*
(*augas_loadfn)(const struct augas_service*);

typedef void
(*augas_unloadfn)(void);

#endif /* AUGAS_MODULE_H */
