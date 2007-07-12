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

    typedef deque<string> tokens;

    struct quit { };

    map<string, void (*)(tokens&)> builtins;
    map<string, tokens> symbols;
    stack<tokens> blocks;

    void
    pop(tokens& args, unsigned n)
    {
        while (n--)
            args.pop_front();
    }

    template<typename T>
    void
    push(tokens& args, const T& arg)
    {
        stringstream os;
        os << arg;
        args.push_front(os.str());
    }

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

    void
    lcase(string& s)
    {
        transform(s.begin(), s.end(), s.begin(), augrt::lcase);
    }

    void
    apply(const string& op, tokens& args)
    {
        map<string, void (*)(tokens&)>
            ::const_iterator it(builtins.find(op));
        if (it == builtins.end())
            throw runtime_error(string("invalid operation: ").append(op));
        it->second(args);
    }

    void
    parse(istream& is, tokens& args)
    {
        string tok;
        while (is >> tok) {
            if (isalpha(tok[0])) {
                lcase(tok);
                apply(tok, args);
            } else
                args.push_front(tok);
        }
    }

    void
    parse(const string& s, tokens& args)
    {
        istringstream is(s);
        parse(is, args);
    }

    void
    printop(tokens& args)
    {
        tokens::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it)
            cout << *it << endl;
    }

    void
    prodop(tokens& args)
    {
        double x(1.0);
        tokens::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it)
            x *= to<double>(*it);
        args.clear();
        push(args, x);
    }

    void
    sumop(tokens& args)
    {
        double x(0.0);
        tokens::const_iterator it(args.begin()), end(args.end());
        for (; it != end; ++it)
            x += to<double>(*it);
        args.clear();
        push(args, x);
    }

    void
    swapop(tokens& args)
    {
        string tmp(args[0]);
        args[0] = args[1];
        args[1] = tmp;
    }

    void
    quitop(tokens& args)
    {
        throw quit();
    }
}

int
main(int argc, char* argv[])
{
    string line;
    tokens args;

    builtins["print"] = printop;
    builtins["prod"] = prodop;
    builtins["sum"] = sumop;
    builtins["swap"] = swapop;
    builtins["quit"] = quitop;

    while (getline(cin, line)) {
        trim(line);
        try {
            parse(line, args);
        } catch (const exception& e) {
            cerr << "error: " << e.what() << endl;
        } catch (const quit& e) {
            break;
        }
    }
    return 0;
}
