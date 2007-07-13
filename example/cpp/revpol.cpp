/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augrtpp.hpp"

#include <cctype>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <stack>
#include <string>

using namespace augrt;
using namespace std;

/*
  :tax begin
    1.175 mul
  end def
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
            return nodes_[size() - (n + 1)];
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
            return nodes_[size() - (n + 1)];
        }
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

        double
        tof() const;

        string
        tos() const;

        void
        print(ostream& os) const;
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
        virtual void
        do_eval(stackv<node>& args)
        {
            retain();
            args.push(node::attach(this));
        }

        virtual double
        do_tof() const = 0;

        virtual string
        do_tos() const = 0;

        virtual void
        do_print(ostream& os) const = 0;

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
        if (ptr_)
            ptr_->do_eval(args);
    }

    double
    node::tof() const
    {
        return ptr_ ? ptr_->do_tof() : 0.0;
    }

    string
    node::tos() const
    {
        string s;
        if (ptr_)
            s = ptr_->do_tos();
        return s;
    }

    void
    node::print(ostream& os) const
    {
        if (ptr_)
            ptr_->do_print(os);
        else
            os << "<null>";
    }

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
                it->eval(args);
        }
        double
        do_tof() const
        {
            throw runtime_error("invalid type: block");
        }
        string
        do_tos() const
        {
            throw runtime_error("invalid type: block");
        }
        void
        do_print(ostream& os) const
        {
            vector<node>::const_iterator it(nodes_.begin()),
                end(nodes_.end());
            for (; it != end; ++it)
                cout << *it << endl;
        }
    };

    class funval : public node_base {
        void (*fun_)(argv&);
        explicit
        funval(void (*fun)(argv&))
            : fun_(fun)
        {
        }
    public:
        virtual
        ~funval()
        {
        }
        static node
        create(void (*fun)(argv&))
        {
            return node::attach(new funval(fun));
        }
        virtual void
        do_eval(stackv<node>& args)
        {
            fun_(args);
        }
        double
        do_tof() const
        {
            throw runtime_error("invalid type: <fun>");
        }
        string
        do_tos() const
        {
            throw runtime_error("invalid type: <fun>");
        }
        void
        do_print(ostream& os) const
        {
            os << "<fun>";
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
        double
        do_tof() const
        {
            return value_;
        }
        string
        do_tos() const
        {
            throw runtime_error("invalid type: <num>");
        }
        void
        do_print(ostream& os) const
        {
            os << value_;
        }
    };

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
        double
        do_tof() const
        {
            return atof(value_.c_str());
        }
        string
        do_tos() const
        {
            return value_;
        }
        void
        do_print(ostream& os) const
        {
            os << value_;
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
        double
        do_tof() const
        {
            throw runtime_error("invalid type: name");
        }
        string
        do_tos() const
        {
            return value_;
        }
        void
        do_print(ostream& os) const
        {
            os << ':' << value_;
        }
    };

    struct begin { };
    struct end { };
    struct quit { };

    map<string, node> defs;

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
        if (!args.empty())
            cout << args.top() << endl;
    }

    node
    createnode(const string& tok)
    {
        if (':' == tok[0])
            return name::create(lcase(tok.substr(1)));

        if (!isalpha(tok[0]))
            return strval::create(tok);

        string lc(lcase(tok));

        if (lc == "begin")
            throw begin();

        if (lc == "end")
            throw end();

        map<string, node>::iterator it(defs.find(lcase(lc)));
        if (it == defs.end())
            throw runtime_error(string("invalid operation: ").append(lc));

        return it->second;
    }

    void
    parse(istream& is, stack<vector<node> >& blocks)
    {
        string tok;
        while (is >> tok) {
            try {
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
    defop(argv& args)
    {
        defs[args[1].tos()] = args[0];
        args.pop(2);
    }

    void
    printop(argv& args)
    {
        argv::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it)
            cout << *it << endl;
    }

    void
    prodop(argv& args)
    {
        double x(1.0);
        argv::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it)
            x *= it->tof();
        args.clear();
        args.push(numval::create(x));
    }

    void
    sumop(argv& args)
    {
        double x(0.0);
        argv::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it)
            x += it->tof();
        args.clear();
        args.push(numval::create(x));
    }

    void
    swapop(argv& args)
    {
        node tmp(args[0]);
        args[0] = args[1];
        args[1] = tmp;
    }

    void
    quitop(argv& args)
    {
        throw quit();
    }
}

int
main(int argc, char* argv[])
{
    defs["def"] = funval::create(defop);
    defs["print"] = funval::create(printop);
    defs["prod"] = funval::create(prodop);
    defs["sum"] = funval::create(sumop);
    defs["swap"] = funval::create(swapop);
    defs["quit"] = funval::create(quitop);

    string line;
    stack<vector<node> > blocks;
    blocks.push(vector<node>());

    while (getline(cin, line)) {
        trim(line);
        try {
            parse(line, blocks);
        } catch (const quit&) {
            break;
        } catch (const exception& e) {
            cerr << "error: " << e.what() << endl;
        }
    }
    return 0;
}
