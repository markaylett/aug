/* -*- c++ -*- */
#ifndef AUG_EVENTOB_H
#define AUG_EVENTOB_H

#include "augobj.h"

#if defined(__cplusplus)

#include "augobjpp.hpp"

# if !defined(AUG_NOTHROW)
#  define AUG_NOTHROW
# endif /* !AUG_NOTHROW */

/* For pointer conversions, see 4.10/2:

   "An rvalue of type 'pointer to cv T,' where T is an object type, can be
   converted to an rvalue of type 'pointer to cv void.' The result of
   converting a 'pointer to cv T' to a 'pointer to cv void' points to the
   start of the storage location where the object of type T resides, as if the
   object is a most derived object (1.8) of type T (that is, not a base class
   subobject)."

   So the void * will point to the beginning of your class B. And since B is
   not guaranteed to start with the POD, you may not get what you want. */

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

AUG_OBJECTDECL(aug_eventob);
struct aug_eventobvtbl {
    AUG_OBJECT(aug_eventob);
    void (*setuser_)(aug_eventob*, aug_object*);
    const char* (*from_)(aug_eventob*);
    const char* (*to_)(aug_eventob*);
    const char* (*type_)(aug_eventob*);
    aug_object* (*user_)(aug_eventob*);
};

#define aug_seteventobuser(obj, user) \
    ((aug_eventob*)obj)->vtbl_->setuser_(obj, user)

#define aug_eventobfrom(obj) \
    ((aug_eventob*)obj)->vtbl_->from_(obj)

#define aug_eventobto(obj) \
    ((aug_eventob*)obj)->vtbl_->to_(obj)

#define aug_eventobtype(obj) \
    ((aug_eventob*)obj)->vtbl_->type_(obj)

#define aug_eventobuser(obj) \
    ((aug_eventob*)obj)->vtbl_->user_(obj)

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_eventob> {
        typedef aug_eventobvtbl vtbl;
        static const char*
        id()
        {
            return aug_eventobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_eventob> eventobref;

    inline void
    seteventobuser(eventobref ref, aug_object* user) AUG_NOTHROW
    {
        aug_eventob* obj(ref.get());
        obj->vtbl_->setuser_(obj, user);
    }

    inline const char*
    eventobfrom(eventobref ref) AUG_NOTHROW
    {
        aug_eventob* obj(ref.get());
        return obj->vtbl_->from_(obj);
    }

    inline const char*
    eventobto(eventobref ref) AUG_NOTHROW
    {
        aug_eventob* obj(ref.get());
        return obj->vtbl_->to_(obj);
    }

    inline const char*
    eventobtype(eventobref ref) AUG_NOTHROW
    {
        aug_eventob* obj(ref.get());
        return obj->vtbl_->type_(obj);
    }

    inline aug_object*
    eventobuser(eventobref ref) AUG_NOTHROW
    {
        aug_eventob* obj(ref.get());
        return obj->vtbl_->user_(obj);
    }

    template <typename T>
    class eventob {

        aug_eventob eventob_;

        eventob(const eventob&);

        eventob&
        operator =(const eventob&);

        static void*
        cast_(aug_eventob* obj, const char* id) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->cast(id).get();
        }
        static int
        incref_(aug_eventob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->incref();
        }
        static int
        decref_(aug_eventob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->decref();
        }
        static void
        setuser_(aug_eventob* obj, aug_object* user) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            impl->seteventobuser(user);
        }
        static const char*
        from_(aug_eventob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->eventobfrom();
        }
        static const char*
        to_(aug_eventob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->eventobto();
        }
        static const char*
        type_(aug_eventob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->eventobtype();
        }
        static aug_object*
        user_(aug_eventob* obj) AUG_NOTHROW
        {
            T* impl = static_cast<T*>(obj->impl_);
            return impl->eventobuser();
        }
        static const aug_eventobvtbl*
        vtbl()
        {
            static const aug_eventobvtbl local = {
                cast_,
                incref_,
                decref_,
                setuser_,
                from_,
                to_,
                type_,
                user_
            };
            return &local;
        }
    public:
        explicit
        eventob(T* impl = 0)
        {
            eventob_.vtbl_ = vtbl();
            eventob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            eventob_.impl_ = impl;
        }
        aug_eventob*
        get()
        {
            return &eventob_;
        }
        operator eventobref()
        {
            return get();
        }
    };

    template <typename T>
    class basic_eventob {
        eventob<basic_eventob<T> > eventob_;
        T impl_;
        unsigned refs_;
        explicit
        basic_eventob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            eventob_.reset(this);
        }
    public:
        objectref
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_eventob>(id))
                return eventob_;
            return null;
        }
        int
        incref() AUG_NOTHROW
        {
            ++refs_;
            return 0;
        }
        int
        decref() AUG_NOTHROW
        {
            if (0 == --refs_)
                delete this;
            return 0;
        }
        void
        seteventobuser(aug_object* user) AUG_NOTHROW
        {
            impl_.seteventobuser(user);
        }
        const char*
        eventobfrom() AUG_NOTHROW
        {
            return impl_.eventobfrom();
        }
        const char*
        eventobto() AUG_NOTHROW
        {
            return impl_.eventobto();
        }
        const char*
        eventobtype() AUG_NOTHROW
        {
            return impl_.eventobtype();
        }
        aug_object*
        eventobuser() AUG_NOTHROW
        {
            return impl_.eventobuser();
        }
        static aug::smartob<aug_eventob>
        create(const T& impl = T())
        {
            basic_eventob* ptr(new basic_eventob(impl));
            return aug::object_attach<aug_eventob>(ptr->eventob_);
        }
    };

    template <typename T>
    class scoped_eventob {
        eventob<scoped_eventob<T> > eventob_;
        T impl_;
    public:
        explicit
        scoped_eventob(const T& impl = T())
            : impl_(impl)
        {
            eventob_.reset(this);
        }
        objectref
        cast(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_eventob>(id))
                return eventob_;
            return null;
        }
        int
        incref() AUG_NOTHROW
        {
            return 0;
        }
        int
        decref() AUG_NOTHROW
        {
            return 0;
        }
        void
        seteventobuser(aug_object* user) AUG_NOTHROW
        {
            impl_.seteventobuser(user);
        }
        const char*
        eventobfrom() AUG_NOTHROW
        {
            return impl_.eventobfrom();
        }
        const char*
        eventobto() AUG_NOTHROW
        {
            return impl_.eventobto();
        }
        const char*
        eventobtype() AUG_NOTHROW
        {
            return impl_.eventobtype();
        }
        aug_object*
        eventobuser() AUG_NOTHROW
        {
            return impl_.eventobuser();
        }
        aug_eventob*
        get()
        {
            return eventob_.get();
        }
        operator eventobref()
        {
            return eventob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_EVENTOB_H */
