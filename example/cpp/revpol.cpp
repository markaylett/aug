/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augrtpp.hpp"

#include <cctype>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <stack>
#include <string>

using namespace augrt;
using namespace std;

/*
  /fib {
  dup dup 1 eq exch 0 eq or not {
  dup 1 sub fib
  exch 2 sub fib
  add
  } if
  } def
*/

#define MEMPOOL_ 0
#define TRACE_ 0

namespace {

    struct begin { };
    struct end { };
    struct quit { };

#if MEMPOOL_
    class mempool {
        const size_t size_;
        stack<void*, vector<void*> > freelist_;
    public:
        ~mempool()
        {
            while (!freelist_.empty()) {
                ::operator delete(freelist_.top());
                freelist_.pop();
            }
        }
        explicit
        mempool(size_t size)
            : size_(size)
        {
        }
        void*
        create()
        {
            void* ptr;
            if (freelist_.empty()) {
                ptr = ::operator new(size_);
            } else {
                ptr = freelist_.top();
                freelist_.pop();
            }
            return ptr;
        }
        void
        destroy(void* ptr)
        {
            freelist_.push(ptr);
        }
    };
#else // !MEMPOOL_
    class mempool {
        const size_t size_;
    public:
        ~mempool()
        {
        }
        explicit
        mempool(size_t size)
            : size_(size)
        {
        }
        void*
        create()
        {
            return ::operator new(size_);
        }
        void
        destroy(void* ptr)
        {
            ::operator delete(ptr);
        }
    };
#endif // !MEMPOOL_

    class num {
        friend ostream&
        operator <<(ostream&, const num&);
        enum { DNUM, LNUM } type_;
        union {
            double d_;
            long l_;
        } u_;
    public:
        num()
            : type_(LNUM)
        {
            u_.l_ = 0;
        }
        num(const num& rhs)
        {
            if (DNUM == (type_ = rhs.type_))
                u_.d_ = rhs.u_.d_;
            else
                u_.l_ = rhs.u_.l_;
        }
        num(double d)
            : type_(DNUM)
        {
            u_.d_ = d;
        }
        num(long l)
            : type_(LNUM)
        {
            u_.l_ = l;
        }
        num&
        operator =(const num& rhs)
        {
            if (DNUM == (type_ = rhs.type_))
                u_.d_ = rhs.u_.d_;
            else
                u_.l_ = rhs.u_.l_;
            return *this;
        }
        num&
        operator =(double d)
        {
            type_ = DNUM;
            u_.d_ = d;
            return *this;
        }
        num&
        operator =(long l)
        {
            type_ = LNUM;
            u_.l_ = l;
            return *this;
        }
        num&
        operator +=(const num& rhs)
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                u_.l_ += rhs.u_.l_;
            else
                *this = tod() + rhs.tod();
            return *this;
        }

        num&
        operator -=(const num& rhs)
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                u_.l_ -= rhs.u_.l_;
            else
                *this = tod() - rhs.tod();
            return *this;
        }
        num&
        operator *=(const num& rhs)
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                u_.l_ *= rhs.u_.l_;
            else
                *this = tod() * rhs.tod();
            return *this;
        }
        num&
        operator /=(const num& rhs)
        {
            *this = tod() / rhs.tod();
            return *this;
        }
        num&
        operator %=(const num& rhs)
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                u_.l_ %= rhs.u_.l_;
            else
                *this = fmod(tod(), rhs.tod());
            return *this;
        }
        num&
        operator ++()
        {
            if (DNUM == type_)
                ++u_.d_;
            else
                ++u_.l_;
            return *this;
        }
        const num
        operator ++(int)
        {
            num prev(*this);
            ++*this;
            return prev;
        }
        num&
        operator --()
        {
            if (DNUM == type_)
                --u_.d_;
            else
                --u_.l_;
            return *this;
        }
        const num
        operator --(int)
        {
            num prev(*this);
            --*this;
            return prev;
        }
        double
        tod() const
        {
            return DNUM == type_ ? u_.d_ : static_cast<double>(u_.l_);
        }
        long
        tol() const
        {
            return LNUM == type_ ? u_.l_ : static_cast<long>(u_.d_);
        }
        bool
        operator <(const num& rhs) const
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                return u_.l_ < rhs.u_.l_;

            return tod() < rhs.tod();
        }
        bool
        operator <=(const num& rhs) const
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                return u_.l_ <= rhs.u_.l_;

            return tod() <= rhs.tod();
        }
        bool
        operator ==(const num& rhs) const
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                return u_.l_ == rhs.u_.l_;

            return tod() == rhs.tod();
        }
        bool
        operator >=(const num& rhs) const
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                return u_.l_ >= rhs.u_.l_;

            return tod() >= rhs.tod();
        }
        bool
        operator >(const num& rhs) const
        {
            if (LNUM == type_ && LNUM == rhs.type_)
                return u_.l_ > rhs.u_.l_;

            return tod() > rhs.tod();
        }
    };

    bool
    operator !=(const num& lhs, const num& rhs)
    {
        return !(lhs == rhs);
    }

    num
    operator +(const num& lhs, const num& rhs)
    {
        num tmp(lhs);
        tmp += rhs;
        return tmp;
    }

    num
    operator -(const num& lhs, const num& rhs)
    {
        num tmp(lhs);
        tmp -= rhs;
        return tmp;
    }

    num
    operator *(const num& lhs, const num& rhs)
    {
        num tmp(lhs);
        tmp *= rhs;
        return tmp;
    }

    num
    operator /(const num& lhs, const num& rhs)
    {
        num tmp(lhs);
        tmp /= rhs;
        return tmp;
    }

    num
    operator %(const num& lhs, const num& rhs)
    {
        num tmp(lhs);
        tmp %= rhs;
        return tmp;
    }

    ostream&
    operator <<(ostream& os, const num& x)
    {
        if (num::DNUM == x.type_)
            os << x.u_.d_;
        else
            os << x.u_.l_;
        return os;
    }

    bool
    getnum(num& n, const string& s)
    {
        char* end;

        long l(strtol(s.c_str(), &end, 10));
        if (end == s.data() + s.size()) {
            n = l;
            return true;
        }

        double d(strtod(s.c_str(), &end));
        if (end == s.data() + s.size()) {
            n = d;
            return true;
        }

        return false;
    }

    template <typename T>
    class stackv {
    public:
        typedef typename vector<T>::value_type value_type;
        typedef typename vector<T>::size_type size_type;
        typedef typename vector<T>::reverse_iterator iterator;
        typedef typename vector<T>::const_reverse_iterator const_iterator;
        typedef typename vector<T>::iterator reverse_iterator;
        typedef typename vector<T>::const_iterator const_reverse_iterator;
    private:
        vector<T>& nodes_;
    public:
        explicit
        stackv(vector<T>& nodes)
            : nodes_(nodes)
        {
        }
        iterator
        begin()
        {
            return nodes_.rbegin();
        }
        iterator
        end()
        {
            return nodes_.rend();
        }
        reverse_iterator
        rbegin()
        {
            return nodes_.begin();
        }
        reverse_iterator
        rend()
        {
            return nodes_.end();
        }
        void
        clear()
        {
            nodes_.clear();
        }
        void
        pop()
        {
            nodes_.pop_back();
        }
        void
        pop(size_type n)
        {
            nodes_.resize(size() - n);
        }
        void
        push(const value_type& x)
        {
            nodes_.push_back(x);
        }
        value_type&
        top()
        {
            return nodes_.back();
        }
        value_type&
        operator [](size_type n)
        {
            return nodes_.at(size() - (n + 1));
        }
        const_iterator
        begin() const
        {
            return nodes_.rbegin();
        }
        const_iterator
        end() const
        {
            return nodes_.rend();
        }
        const_reverse_iterator
        rbegin() const
        {
            return nodes_.begin();
        }
        const_reverse_iterator
        rend() const
        {
            return nodes_.end();
        }
        bool
        empty() const
        {
            return nodes_.empty();
        }
        size_type
        size() const
        {
            return nodes_.size();
        }
        const value_type&
        top() const
        {
            return nodes_.back();
        }
        const value_type&
        operator [](size_type n) const
        {
            return nodes_.at(size() - (n + 1));
        }
    };

    enum cmptype {
        EQ,
        GT,
        LT,
        NEQ
    };

    enum nodetype {
        BRANCH,
        FUN,
        NAME,
        NOOP,
        NUM,
        REF,
        STR
    };

    class node_base;

    class node {
        node_base* ptr_;
        node(node_base* ptr)
            : ptr_(ptr)
        {
        }
    public:
        ~node();

        node()
            : ptr_(0)
        {
        }

        node(const node& rhs);

        node&
        operator =(const node& rhs)
        {
            node tmp(rhs);
            swap(tmp);
            return *this;
        }
        void
        swap(node& rhs)
        {
            std::swap(ptr_, rhs.ptr_);
        }
        static node
        attach(node_base* ptr)
        {
            return node(ptr);
        }
        void
        eval(bool fork, stackv<node>& args, deque<node>& tail);

        cmptype
        cmp(const node& rhs) const;

        const node_base*
        get() const
        {
            return ptr_;
        }

        bool
        isconst(stackv<node>& args) const;

        void
        print(ostream& os) const;

        bool
        tobool() const;

        num
        tonum() const;

        string
        tostr() const;

        nodetype
        type() const;
    };

    ostream&
    operator <<(ostream& os, const node& x)
    {
        x.print(os);
        return os;
    }

    typedef stackv<node> argv;
    map<nodetype, vector<node> > freelist_;

    class node_base {
        friend class node;
        mutable unsigned refs_;
        void
        release() const
        {
            if (0 == --refs_)
                delete this;
        }
        void
        retain() const
        {
            ++refs_;
        }
    protected:

        virtual void
        do_eval(bool fork, stackv<node>& args, deque<node>& tail) = 0;

        virtual cmptype
        do_cmp(const node& rhs) const = 0;

        virtual bool
        do_isconst(argv& args) const = 0;

        virtual void
        do_print(ostream& os) const = 0;

        virtual bool
        do_tobool() const = 0;

        virtual num
        do_tonum() const = 0;

        virtual string
        do_tostr() const = 0;

        virtual nodetype
        do_type() const = 0;

    public:
        virtual
        ~node_base()
        {
        }
        node_base()
            : refs_(1)
        {
        }
    };

    node::~node()
    {
        if (ptr_)
            ptr_->release();
    }

    node::node(const node& rhs)
        : ptr_(rhs.ptr_)
    {
        if (ptr_)
            ptr_->retain();
    }

    void
    node::eval(bool fork, stackv<node>& args, deque<node>& tail)
    {
#if TRACE_
        if (1 < args.size())
            cout << args[1] << ' ' << args[0] << ' ';
        else if (!args.empty())
            cout << args[0] << ' ';
        print(cout);
        cout << endl;
#endif // TRACE_
        if (ptr_)
            ptr_->do_eval(fork, args, tail);
    }

    cmptype
    node::cmp(const node& rhs) const
    {
        if (!ptr_)
            return rhs.ptr_ ? NEQ : EQ;

        return ptr_->do_cmp(rhs);
    }

    bool
    node::isconst(stackv<node>& args) const
    {
        return false;
    }

    void
    node::print(ostream& os) const
    {
        if (ptr_)
            ptr_->do_print(os);
        else
            os << "<nop>";
    }

    bool
    node::tobool() const
    {
        return ptr_ ? ptr_->do_tobool() : false;
    }

    num
    node::tonum() const
    {
        return ptr_ ? ptr_->do_tonum() : 0L;
    }

    string
    node::tostr() const
    {
        string s;
        if (ptr_)
            s = ptr_->do_tostr();
        return s;
    }

    nodetype
    node::type() const
    {
        return ptr_ ? ptr_->do_type() : NOOP;
    }

    void
    node_base::do_eval(bool fork, stackv<node>& args, deque<node>& tail)
    {
        retain();
        args.push(node::attach(this));
    }

    cmptype
    node_base::do_cmp(const node& rhs) const
    {
        return this == rhs.get() ? EQ : NEQ;
    }

    map<string, node> defs;

    class branch : public node_base {
        static mempool mempool_;
        vector<node> nodes_;
        explicit
        branch(const vector<node>& nodes)
            : nodes_(nodes)
        {
        }
    public:
        ~branch()
        {
        }
        static void*
        operator new(size_t size)
        {
            assert(sizeof(test) == size);
            return mempool_.create();
        }
        static void
        operator delete(void* ptr)
        {
            mempool_.destroy(ptr);
        }
        static node
        create(const vector<node>& nodes)
        {
            return node::attach(new branch(nodes));
        }
        void
        do_eval(bool fork, stackv<node>& args, deque<node>& tail)
        {
            if (fork)
                copy(nodes_.rbegin(), nodes_.rend(), front_inserter(tail));
            else
                node_base::do_eval(fork, args, tail);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_base::do_cmp(rhs);
        }
        bool
        do_isconst(argv& args) const
        {
            return false;
        }
        void
        do_print(ostream& os) const
        {
            os << '{';
            vector<node>::const_iterator it(nodes_.begin()),
                end(nodes_.end());
            for (; it != end; ++it) {
                os << ' ';
                it->print(os);
            }
            os << " }";
        }
        bool
        do_tobool() const
        {
            throw runtime_error("invalid type: <branch>");
        }
        num
        do_tonum() const
        {
            throw runtime_error("invalid type: <branch>");
        }
        string
        do_tostr() const
        {
            throw runtime_error("invalid type: <branch>");
        }
        nodetype
        do_type() const
        {
            return BRANCH;
        }
    };

    mempool branch::mempool_(sizeof(branch));

    class ref : public node_base {
        static mempool mempool_;
        string name_;
        node node_;
        bool bound_;
        explicit
        ref(const string& name)
            : name_(name),
              bound_(false)
        {
            map<string, node>::iterator it(defs.find(name));
            if (it != defs.end()) {
                node_ = it->second;
                bound_ = true;
            }
        }
    public:
        ~ref()
        {
        }
        static void*
        operator new(size_t size)
        {
            assert(sizeof(test) == size);
            return mempool_.create();
        }
        static void
        operator delete(void* ptr)
        {
            mempool_.destroy(ptr);
        }
        static node
        create(const string& name)
        {
            return node::attach(new ref(name));
        }
        void
        do_eval(bool fork, stackv<node>& args, deque<node>& tail)
        {
            if (!bound_) {
                map<string, node>::iterator it(defs.find(name_));
                if (it != defs.end())
                    node_ = it->second;
                bound_ = true;
            }
            node_.eval(true, args, tail);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_.cmp(rhs);
        }
        bool
        do_isconst(argv& args) const
        {
            return false;
        }
        void
        do_print(ostream& os) const
        {
            os << name_;
        }
        bool
        do_tobool() const
        {
            return node_.tobool();
        }
        num
        do_tonum() const
        {
            return node_.tonum();
        }
        string
        do_tostr() const
        {
            return node_.tostr();
        }
        nodetype
        do_type() const
        {
            return REF;
        }
    };

    mempool ref::mempool_(sizeof(ref));

    class fun : public node_base {
        static mempool mempool_;
        void (*fn_)(argv&, deque<node>&);
        const int argc_;
        fun(void (*fn)(argv&, deque<node>&), int argc)
            : fn_(fn),
              argc_(argc)
        {
        }
    public:
        ~fun()
        {
        }
        static void*
        operator new(size_t size)
        {
            assert(sizeof(test) == size);
            return mempool_.create();
        }
        static void
        operator delete(void* ptr)
        {
            mempool_.destroy(ptr);
        }
        static node
        create(void (*fn)(argv&, deque<node>&), int argc = -1)
        {
            return node::attach(new fun(fn, argc));
        }
        void
        do_eval(bool fork, stackv<node>& args, deque<node>& tail)
        {
            fn_(args, tail);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_base::do_cmp(rhs);
        }
        bool
        do_isconst(argv& args) const
        {
            if (-1 == argc_ || args.size() < (unsigned)argc_)
                return false;

            return true;
        }
        void
        do_print(ostream& os) const
        {
            os << "<fun>";
        }
        bool
        do_tobool() const
        {
            throw runtime_error("invalid type: <fun>");
        }
        num
        do_tonum() const
        {
            throw runtime_error("invalid type: <fun>");
        }
        string
        do_tostr() const
        {
            throw runtime_error("invalid type: <fun>");
        }
        nodetype
        do_type() const
        {
            return FUN;
        }
    };

    mempool fun::mempool_(sizeof(fun));

    class name : public node_base {
        static mempool mempool_;
        string value_;
        explicit
        name(const string& value)
            : value_(value)
        {
        }
    public:
        ~name()
        {
        }
        static void*
        operator new(size_t size)
        {
            assert(sizeof(test) == size);
            return mempool_.create();
        }
        static void
        operator delete(void* ptr)
        {
            mempool_.destroy(ptr);
        }
        static node
        create(const string& value)
        {
            return node::attach(new name(value));
        }
        void
        do_eval(bool fork, stackv<node>& args, deque<node>& tail)
        {
            return node_base::do_eval(fork, args, tail);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_base::do_cmp(rhs);
        }
        bool
        do_isconst(argv& args) const
        {
            return false;
        }
        void
        do_print(ostream& os) const
        {
            os << '/' << value_;
        }
        bool
        do_tobool() const
        {
            return true;
        }
        num
        do_tonum() const
        {
            throw runtime_error("invalid type: <name>");
        }
        string
        do_tostr() const
        {
            return value_;
        }
        nodetype
        do_type() const
        {
            return NAME;
        }
    };

    mempool name::mempool_(sizeof(name));

    class numval : public node_base {
        static mempool mempool_;
        num value_;
        explicit
        numval(const num& value)
            : value_(value)
        {
        }
    public:
        ~numval()
        {
        }
        static void*
        operator new(size_t size)
        {
            assert(sizeof(test) == size);
            return mempool_.create();
        }
        static void
        operator delete(void* ptr)
        {
            mempool_.destroy(ptr);
        }
        static node
        create(const num& value)
        {
            return node::attach(new numval(value));
        }
        void
        do_eval(bool fork, stackv<node>& args, deque<node>& tail)
        {
            return node_base::do_eval(fork, args, tail);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            if (NUM != rhs.type())
                return NEQ;

            num n(rhs.tonum());
            if (value_ < n)
                return LT;

            if (value_ == n)
                return EQ;

            return GT;
        }
        bool
        do_isconst(argv& args) const
        {
            return false;
        }
        void
        do_print(ostream& os) const
        {
            os << value_;
        }
        bool
        do_tobool() const
        {
            return 0L != value_;
        }
        num
        do_tonum() const
        {
            return value_;
        }
        string
        do_tostr() const
        {
            stringstream ss;
            ss << value_;
            return ss.str();
        }
        nodetype
        do_type() const
        {
            return NUM;
        }
    };

    mempool numval::mempool_(sizeof(numval));

    class strval : public node_base {
        static mempool mempool_;
        string value_;
        explicit
        strval(const string& value)
            : value_(value)
        {
        }
    public:
        ~strval()
        {
        }
        static void*
        operator new(size_t size)
        {
            assert(sizeof(test) == size);
            return mempool_.create();
        }
        static void
        operator delete(void* ptr)
        {
            mempool_.destroy(ptr);
        }
        static node
        create(const string& value)
        {
            return node::attach(new strval(value));
        }
        void
        do_eval(bool fork, stackv<node>& args, deque<node>& tail)
        {
            return node_base::do_eval(fork, args, tail);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            if (STR != rhs.type())
                return NEQ;

            int cmp(value_.compare(rhs.tostr()));
            if (cmp < 0)
                return LT;

            if (0 == cmp)
                return EQ;

            return GT;
        }
        bool
        do_isconst(argv& args) const
        {
            return false;
        }
        void
        do_print(ostream& os) const
        {
            os << value_;
        }
        bool
        do_tobool() const
        {
            return !value_.empty();
        }
        num
        do_tonum() const
        {
            num n;
            getnum(n, value_.c_str());
            return n;
        }
        string
        do_tostr() const
        {
            return value_;
        }
        nodetype
        do_type() const
        {
            return STR;
        }
    };

    mempool strval::mempool_(sizeof(strval));

    node zeroval(numval::create(0L));
    node oneval(numval::create(1L));

    string
    lcase(const string& s)
    {
        string lc;
        transform(s.begin(), s.end(), back_inserter(lc), augrt::lcase);
        return lc;
    }

    void
    eval(argv& args, node& head)
    {
        deque<node> tail;
        head.eval(false, args, tail);
        while (!tail.empty()) {

            head = tail.front();
            tail.pop_front();
            head.eval(false, args, tail);
        }
    }

    void
    eval(vector<node>& nodes, node& head)
    {
        argv args(nodes);
        eval(args, head);
    }

    bool
    isconst(vector<node>& nodes, const node& head)
    {
        argv args(nodes);
        return head.isconst(args);
    }

    node
    literal(const string& tok)
    {
        num n;
        if (getnum(n, tok))
            return numval::create(n);

        return strval::create(tok);
    }

    node
    createnode(const string& tok)
    {
        if ('/' == tok[0])
            return name::create(lcase(tok.substr(1)));

        if (tok == "{")
            throw begin();

        if (tok == "}")
            throw end();

        if (!isalpha(tok[0]))
            return literal(tok);

        return ref::create(lcase(tok));
    }

    void
    parse(istream& is, stack<vector<node> >& branches)
    {
        string tok;
        while (is >> tok) {
            try {
                if ('#' == tok[0])
                    break;
                node head(createnode(tok));
                if (1 == branches.size() || isconst(branches.top(), head))
                    eval(branches.top(), head);
                else
                    branches.top().push_back(head);
            } catch (const begin&) {
                branches.push(vector<node>());
            } catch (const end&) {
                node x(branch::create(branches.top()));
                branches.pop();
                branches.top().push_back(x);
            }
        }
    }

    void
    parse(const string& s, stack<vector<node> >& branches)
    {
        istringstream is(s);
        parse(is, branches);
    }

    void
    addfun(argv& args, deque<node>& tail)
    {
        args[1] = numval::create(args[1].tonum() + args[0].tonum());
        args.pop(1);
    }

    void
    andfun(argv& args, deque<node>& tail)
    {
        args[1] = args[1].tobool() && args[0].tobool() ? oneval : zeroval;
        args.pop(1);
    }

    void
    clearfun(argv& args, deque<node>& tail)
    {
        argv::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it)
            if (NOOP == it->type()) {
                ++it;
                break;
            }
        args.pop(distance(static_cast<const argv&>(args).begin(), it));
    }

    void
    decfun(argv& args, deque<node>& tail)
    {
        args[0] = numval::create(--args[0].tonum());
    }

    void
    deffun(argv& args, deque<node>& tail)
    {
        defs[args[1].tostr()] = args[0];
        args.pop(2);
    }

    void
    divfun(argv& args, deque<node>& tail)
    {
        args[1] = numval::create(args[1].tonum() / args[0].tonum());
        args.pop(1);
    }

    void
    dupfun(argv& args, deque<node>& tail)
    {
        args.push(args[0]);
    }

    void
    eqfun(argv& args, deque<node>& tail)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args[1] = EQ == cmp ? oneval : zeroval;
        args.pop(1);
    }

    void
    gefun(argv& args, deque<node>& tail)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args[1] = EQ == cmp || GT == cmp ? oneval : zeroval;
        args.pop(1);
    }

    void
    gtfun(argv& args, deque<node>& tail)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(GT == cmp ? oneval : zeroval);
    }

    void
    iffun(argv& args, deque<node>& tail)
    {
        bool expr(args[1].tobool());
        node b1(args[0]);
        args.pop(2);
        if (expr)
            b1.eval(true, args, tail);
    }

    void
    ifelsefun(argv& args, deque<node>& tail)
    {
        bool expr(args[2].tobool());
        node b1(args[1]);
        node b2(args[0]);
        args.pop(3);
        if (expr) {
            b1.eval(true, args, tail);
        } else {
            b2.eval(true, args, tail);
        }
    }

    void
    incfun(argv& args, deque<node>& tail)
    {
        args[0] = numval::create(++args[0].tonum());
    }

    void
    indexfun(argv& args, deque<node>& tail)
    {
        args.push(args[args[0].tonum().tol()]);
    }

    void
    lefun(argv& args, deque<node>& tail)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(EQ == cmp || LT == cmp ? oneval : zeroval);
    }

    void
    ltfun(argv& args, deque<node>& tail)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(LT == cmp ? oneval : zeroval);
    }

    void
    neqfun(argv& args, deque<node>& tail)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(EQ != cmp ? oneval : zeroval);
    }

    void
    nopfun(argv& args, deque<node>& tail)
    {
        args.push(node());
    }

    void
    notfun(argv& args, deque<node>& tail)
    {
        node ret(args[0].tobool() ? zeroval : oneval);
        args.pop(1);
        args.push(ret);
    }

    void
    orfun(argv& args, deque<node>& tail)
    {
        node ret(args[1].tobool() || args[0].tobool() ? oneval : zeroval);
        args.pop(2);
        args.push(ret);
    }

    void
    popfun(argv& args, deque<node>& tail)
    {
        args.pop();
    }

    void
    modfun(argv& args, deque<node>& tail)
    {
        node ret(numval::create(args[1].tonum() % args[0].tonum()));
        args.pop(2);
        args.push(ret);
    }

    void
    mulfun(argv& args, deque<node>& tail)
    {
        node ret(numval::create(args[1].tonum() * args[0].tonum()));
        args.pop(2);
        args.push(ret);
    }

    void
    printfun(argv& args, deque<node>& tail)
    {
        argv::const_iterator it(args.begin()), end(args.end());
        for (unsigned i(0); it != end; ++it)
            cout << i++ << ": " << *it << endl;
    }

    void
    prodfun(argv& args, deque<node>& tail)
    {
        num prod(1L);
        argv::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it) {
            if (NOOP == it->type()) {
                ++it;
                break;
            }
            prod *= it->tonum();
        }
        args.pop(distance(static_cast<const argv&>(args).begin(), it));
        args.push(numval::create(prod));
    }

    void
    sumfun(argv& args, deque<node>& tail)
    {
        num sum(0L);
        argv::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it) {
            if (NOOP == it->type()) {
                ++it;
                break;
            }
            sum += it->tonum();
        }
        args.pop(distance(static_cast<const argv&>(args).begin(), it));
        args.push(numval::create(sum));
    }

    void
    subfun(argv& args, deque<node>& tail)
    {
        node ret(numval::create(args[1].tonum() - args[0].tonum()));
        args.pop(2);
        args.push(ret);
    }

    void
    exchfun(argv& args, deque<node>& tail)
    {
        node tmp(args[0]);
        args[0] = args[1];
        args[1] = tmp;
    }

    void
    quitfun(argv& args, deque<node>& tail)
    {
        throw quit();
    }
}

int
main(int argc, char* argv[])
{
    defs["add"] = fun::create(addfun);
    defs["and"] = fun::create(andfun);
    defs["clear"] = fun::create(clearfun);
    defs["dec"] = fun::create(decfun);
    defs["def"] = fun::create(deffun);
    defs["div"] = fun::create(divfun);
    defs["dup"] = fun::create(dupfun);
    defs["eq"] = fun::create(eqfun);
    defs["ge"] = fun::create(gefun);
    defs["gt"] = fun::create(gtfun);
    defs["if"] = fun::create(iffun);
    defs["ifelse"] = fun::create(ifelsefun);
    defs["inc"] = fun::create(incfun);
    defs["index"] = fun::create(indexfun);
    defs["le"] = fun::create(lefun);
    defs["lt"] = fun::create(ltfun);
    defs["neq"] = fun::create(neqfun);
    defs["nop"] = fun::create(nopfun);
    defs["not"] = fun::create(notfun);
    defs["or"] = fun::create(orfun);
    defs["pop"] = fun::create(popfun);
    defs["print"] = fun::create(printfun);
    defs["prod"] = fun::create(prodfun);
    defs["mod"] = fun::create(modfun);
    defs["mul"] = fun::create(mulfun);
    defs["sub"] = fun::create(subfun);
    defs["sum"] = fun::create(sumfun);
    defs["exch"] = fun::create(exchfun);
    defs["quit"] = fun::create(quitfun);

    string line;
    stack<vector<node> > branches;
    branches.push(vector<node>());

    while (getline(cin, line)) {
        trim(line);
        if (line.empty())
            continue;
        try {
            parse(line, branches);
#if TRACE_
            if (1 == branches.size() && !branches.top().empty())
                cout << "top: " << branches.top().back() << endl;
#endif // TRACE_
        } catch (const quit&) {
            break;
        } catch (const exception& e) {
            cerr << "error: " << e.what() << endl;
        }
    }
    return 0;
}
