/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augrtpp.hpp"

#include <cctype>
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

namespace {

    template<typename T>
    T
    to(const string& tok)
    {
        istringstream is(tok);
        T x = T();
        if (!(is >> x))
            throw runtime_error(string("invalid type: ").append(tok));
        return x;
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
        BLOCK,
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
        eval(stackv<node>& args);

        cmptype
        cmp(const node& rhs) const;

        const node_base*
        get() const
        {
            return ptr_;
        }

        void
        print(ostream& os) const;

        bool
        tobool() const;

        double
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
        do_eval(stackv<node>& args) = 0;

        virtual cmptype
        do_cmp(const node& rhs) const = 0;

        virtual void
        do_print(ostream& os) const = 0;

        virtual bool
        do_tobool() const = 0;

        virtual double
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
    node::eval(stackv<node>& args)
    {
        if (1 < args.size())
            cout << args[1] << ' ' << args[0] << ' ';
        else if (!args.empty())
            cout << args[0] << ' ';
        print(cout);
        cout << endl;
        if (ptr_)
            ptr_->do_eval(args);
    }

    cmptype
    node::cmp(const node& rhs) const
    {
        if (!ptr_)
            return rhs.ptr_ ? NEQ : EQ;

        return ptr_->do_cmp(rhs);
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

    double
    node::tonum() const
    {
        return ptr_ ? ptr_->do_tonum() : 0.0;
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
    node_base::do_eval(stackv<node>& args)
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

    class block : public node_base {
        vector<node> nodes_;
        explicit
        block(const vector<node>& nodes)
            : nodes_(nodes)
        {
        }
    public:
        virtual
        ~block()
        {
        }
        static node
        create(const vector<node>& nodes)
        {
            return node::attach(new block(nodes));
        }
        virtual void
        do_eval(stackv<node>& args)
        {
            vector<node>::iterator it(nodes_.begin()), end(nodes_.end());
            for (; it != end; ++it)
                if (BLOCK == it->type())
                    args.push(*it);
                else
                    it->eval(args);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_base::do_cmp(rhs);
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
            throw runtime_error("invalid type: <block>");
        }
        double
        do_tonum() const
        {
            throw runtime_error("invalid type: <block>");
        }
        string
        do_tostr() const
        {
            throw runtime_error("invalid type: <block>");
        }
        nodetype
        do_type() const
        {
            return BLOCK;
        }
    };

    class ref : public node_base {
        string name_;
        node node_;
        explicit
        ref(const string& name)
            : name_(name)
        {
        }
    public:
        virtual
        ~ref()
        {
        }
        virtual void
        do_eval(stackv<node>& args)
        {
            map<string, node>::iterator it(defs.find(name_));
            node_ = it == defs.end() ? node() : it->second;
            node_.eval(args);
        }
        static node
        create(const string& name)
        {
            return node::attach(new ref(name));
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_.cmp(rhs);
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
        double
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

    class fun : public node_base {
        void (*fn_)(argv&);
        explicit
        fun(void (*fn)(argv&))
            : fn_(fn)
        {
        }
    public:
        virtual
        ~fun()
        {
        }
        static node
        create(void (*fn)(argv&))
        {
            return node::attach(new fun(fn));
        }
        virtual void
        do_eval(stackv<node>& args)
        {
            fn_(args);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_base::do_cmp(rhs);
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
        double
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

    class name : public node_base {
        string value_;
        explicit
        name(const string& value)
            : value_(value)
        {
        }
    public:
        virtual
        ~name()
        {
        }
        static node
        create(const string& value)
        {
            return node::attach(new name(value));
        }
        void
        do_eval(stackv<node>& args)
        {
            return node_base::do_eval(args);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            return node_base::do_cmp(rhs);
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
        double
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

    class numval : public node_base {
        double value_;
        explicit
        numval(double value)
            : value_(value)
        {
        }
    public:
        virtual
        ~numval()
        {
        }
        static node
        create(double value)
        {
            return node::attach(new numval(value));
        }
        void
        do_eval(stackv<node>& args)
        {
            return node_base::do_eval(args);
        }
        cmptype
        do_cmp(const node& rhs) const
        {
            if (NUM != rhs.type())
                return NEQ;

            double num(rhs.tonum());
            if (value_ < num)
                return LT;

            if (value_ == num)
                return EQ;

            return GT;
        }
        void
        do_print(ostream& os) const
        {
            os << value_;
        }
        bool
        do_tobool() const
        {
            return 0.0 != value_;
        }
        double
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

    node zeroval(numval::create(0.0));
    node oneval(numval::create(1.0));

    class strval : public node_base {
        string value_;
        explicit
        strval(const string& value)
            : value_(value)
        {
        }
    public:
        virtual
        ~strval()
        {
        }
        static node
        create(const string& value)
        {
            return node::attach(new strval(value));
        }
        void
        do_eval(stackv<node>& args)
        {
            return node_base::do_eval(args);
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
        double
        do_tonum() const
        {
            return atof(value_.c_str());
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

    struct begin { };
    struct end { };
    struct quit { };

    string
    lcase(const string& s)
    {
        string lc;
        transform(s.begin(), s.end(), back_inserter(lc), augrt::lcase);
        return lc;
    }

    void
    eval(node& head, vector<node>& nodes)
    {
        argv args(nodes);
        head.eval(args);
    }

    node
    literal(const string& tok)
    {
		char* end;
		double d(strtod(tok.c_str(), &end));
        return tok.c_str() + tok.size() == end
            ? numval::create(d)
            : strval::create(tok);
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
    parse(istream& is, stack<vector<node> >& blocks)
    {
        string tok;
        while (is >> tok) {
            try {
                if ('#' == tok[0])
                    break;
                node x(createnode(tok));
                if (1 == blocks.size())
                    eval(x, blocks.top());
                else
                    blocks.top().push_back(x);
            } catch (const begin&) {
                blocks.push(vector<node>());
            } catch (const end&) {
                node x(block::create(blocks.top()));
                blocks.pop();
                blocks.top().push_back(x);
            }
        }
    }

    void
    parse(const string& s, stack<vector<node> >& blocks)
    {
        istringstream is(s);
        parse(is, blocks);
    }

    void
    addfun(argv& args)
    {
        node ret(numval::create(args[1].tonum() + args[0].tonum()));
        args.pop(2);
        args.push(ret);
    }

    void
    andfun(argv& args)
    {
        node ret(args[1].tobool() && args[0].tobool() ? oneval : zeroval);
        args.pop(2);
        args.push(ret);
    }

    void
    clearfun(argv& args)
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
    deffun(argv& args)
    {
        defs[args[1].tostr()] = args[0];
        args.pop(2);
    }

    void
    divfun(argv& args)
    {
        node ret(numval::create(args[1].tonum() / args[0].tonum()));
        args.pop(2);
        args.push(ret);
    }

    void
    dupfun(argv& args)
    {
        args.push(args[0]);
    }

    void
    eqfun(argv& args)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(EQ == cmp ? oneval : zeroval);
    }

    void
    gefun(argv& args)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(EQ == cmp || GT == cmp ? oneval : zeroval);
    }

    void
    gtfun(argv& args)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(GT == cmp ? oneval : zeroval);
    }

    void
    iffun(argv& args)
    {
        bool expr(args[1].tobool());
        node b1(args[0]);
        args.pop(2);
        if (expr)
            b1.eval(args);
    }

    void
    ifelsefun(argv& args)
    {
        bool expr(args[2].tobool());
        node b1(args[1]);
        node b2(args[0]);
        args.pop(3);
        if (expr) {
            b1.eval(args);
        } else {
            b2.eval(args);
        }
    }

    void
    indexfun(argv& args)
    {
        args.push(args[static_cast<unsigned>(args[0].tonum())]);
    }

    void
    lefun(argv& args)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(EQ == cmp || LT == cmp ? oneval : zeroval);
    }

    void
    ltfun(argv& args)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(LT == cmp ? oneval : zeroval);
    }

    void
    neqfun(argv& args)
    {
        cmptype cmp(args[1].cmp(args[0]));
        args.pop(2);
        args.push(EQ != cmp ? oneval : zeroval);
    }

    void
    nopfun(argv& args)
    {
        args.push(node());
    }

    void
    notfun(argv& args)
    {
        node ret(args[0].tobool() ? zeroval : oneval);
        args.pop(1);
        args.push(ret);
    }

    void
    orfun(argv& args)
    {
        node ret(args[1].tobool() || args[0].tobool() ? oneval : zeroval);
        args.pop(2);
        args.push(ret);
    }

    void
    popfun(argv& args)
    {
        args.pop();
    }

    void
    mulfun(argv& args)
    {
        node ret(numval::create(args[1].tonum() * args[0].tonum()));
        args.pop(2);
        args.push(ret);
    }

    void
    printfun(argv& args)
    {
        argv::const_iterator it(args.begin()), end(args.end());
        for (unsigned i(0); it != end; ++it)
            cout << i++ << ": " << *it << endl;
    }

    void
    prodfun(argv& args)
    {
        double prod(1.0);
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
    sumfun(argv& args)
    {
        double sum(0.0);
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
    subfun(argv& args)
    {
        node ret(numval::create(args[1].tonum() - args[0].tonum()));
        args.pop(2);
        args.push(ret);
    }

    void
    exchfun(argv& args)
    {
        node tmp(args[0]);
        args[0] = args[1];
        args[1] = tmp;
    }

    void
    quitfun(argv& args)
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
    defs["def"] = fun::create(deffun);
    defs["div"] = fun::create(divfun);
    defs["dup"] = fun::create(dupfun);
    defs["eq"] = fun::create(eqfun);
    defs["ge"] = fun::create(gefun);
    defs["gt"] = fun::create(gtfun);
    defs["if"] = fun::create(iffun);
    defs["ifelse"] = fun::create(ifelsefun);
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
    defs["mul"] = fun::create(mulfun);
    defs["sub"] = fun::create(subfun);
    defs["sum"] = fun::create(sumfun);
    defs["exch"] = fun::create(exchfun);
    defs["quit"] = fun::create(quitfun);

    string line;
    stack<vector<node> > blocks;
    blocks.push(vector<node>());

    while (getline(cin, line)) {
        trim(line);
        if (line.empty())
            continue;
        try {
            parse(line, blocks);
            if (1 == blocks.size() && !blocks.top().empty())
                cout << "top: " << blocks.top().back() << endl;
        } catch (const quit&) {
            break;
        } catch (const exception& e) {
            cerr << "error: " << e.what() << endl;
        }
    }
    return 0;
}
