% modtest: An example Augas module by Mark Aylett

\nocon % omit table of contents
\datethis % print date on listing

\def\AUG/{{\sc AUG}}
\def\AUGAS/{{\sc AUGAS}}
\def\CYGWIN/{{\sc CYGWIN}}
\def\LINUX/{{\sc LINUX}}
\def\POP3/{{\sc POP3}}
\def\PYTHON/{{\sc PYTHON}}
\def\SMTP/{{\sc SMTP}}
\def\WINDOWS/{{\sc WINDOWS}}

@s std int @s string int

@s augas int
@s basic_factory int
@s basic_module int
@s basic_serv int
@s object int
@s serv int
@s serv_base int
@s user int

@f line normal

@* Introduction.

\AUGAS/ is a network application server.  \AUGAS/ manages many common
and error-prone tasks associated with network servers.

\AUGAS/ provides a host environment to Modules.  Modules are
physical components that are dynamically loaded into the the application
server at run-time.  Each Module exposes one or more services.  Services are
wired together at configuration-time, rather than compile-time.

Such a system allows features common to many services to be implemented in
Modules.  The \PYTHON/ Module is an example of this, it allows services to be
implemented in \PYTHON/.  The \PYTHON/ module exposes the \AUGAS/'s host
environment as a \PYTHON/ module (not to be confused with an \AUGAS/ Module).

\AUGAS/ helps to promote component rather than source-level
reuse, and presents a uniform interface to system administrators, regardless
of the services provided.

@ \AUGAS/ runs natively on a variety of OSes, including \LINUX/ and
\WINDOWS/.  On \WINDOWS/, \AUGAS/ does not require a porting layer, such as \CYGWIN/,
to operate.

Where appropriate, \AUGAS/ adheres to the conventions of the target platform.
D\ae monisation, for example, takes the form of an NT service on \WINDOWS/.
However, from a sys-admin perspective, the interface remains the same.  The
following command can be used to start the service from a command window:

\yskip\.{C:\\> augasd -f augasd.conf start}

@ \AUGAS/ uses an event-based model to de-multiplex activity on signal,
socket, timer and custom event objects.  All module calls are dispatched from
the event thread (similar to a UI thread).  A multi-threaded environment is
not imposed upon a service by the host environment.

Internally, the application operates on a single thread.  Services are,
however, free to select a threading model best suited to their needs.  They
may, for example, use worker threads to handle CPU-intensive tasks.

Services can interact with one another using custom events.  In a
multi-language environment, a \PYTHON/ service could delegate tasks to a
service which happens to be implemented in a different language.

@* Sample Module.

In the sections below, a module is built in \CPLUSPLUS/ that:

\yskip\item{$\bullet$} exposes a TCP service
\item{$\bullet$} reverses lines sent from client
\item{$\bullet$} times-out when a client is inactive

@c
@<include augas header@>@/
@<implement service@>@/
@<declare export table@>

@ The \.{<augaspp.hpp>} provides a set of utiliy functions and classes
designed to facilitate module implementations in \CPLUSPLUS/.  Modules can,
however, just as easily be written in \CEE/.  A \CEE/ implementation would use
the \.{<augas.h>} header.

@<include...@>=
#include <augaspp.hpp>

@ Services must implement the |serv_base| interface.  Stub implementations to
most of the pure virtual functions are provided by |basic_serv|.  For
convenience, |serv| is derived from |basic_serv|.

@<implement...@>=
using namespace augas;
using namespace std;@/

namespace {@/
  @<request handler@>@;
  struct serv : basic_serv {@/
  @<start service@>@/
  @<accept connection@>@/
  @<cleanup on disconnect@>@/
  @<process data@>@/
  @<handle timer expiry@>@/
  };@/
}

@ Here is the main program.

@<declare...@>=
typedef basic_module<basic_factory<serv> > module;@/
AUGAS_MODULE(module::init, module::term)

@ TODO

@<start...@>=
bool
do_start(const char* sname)
{
  writelog(AUGAS_LOGINFO, "starting service [%s]", sname);
  const char* port = augas::getenv("service.echo.serv");
  if (!port)
    return false;
  tcplisten(sname, "0.0.0.0", port);
  return true;
}

@ A buffer is assigned in the socket's user state.  The buffer is stores
incomplete line data received from the client.  Timers are especially useful
for implementing heartbeat mechanisms common to many protocols.

@<accept...@>=
bool
do_accept(object& sock, const char* addr, unsigned short port)
{
  sock.setuser(new string());
  send(sock, "hello\r\n", 7);
  setrwtimer(sock, 15000, AUGAS_TIMRD);
  return true;
}

@ Delete the |string| buffer associated with the connection.

@<cleanup...@>=
void
do_closed(const object& sock)
{
  delete sock.user<string>();
}

@ |tok| is used to buffer incomplete lines between calls to |do_data|.  The
|tokenise()| function appends new data to the back of |tok|.  Each complete
line is processed by the |echoline| functor.

@<process data...@>=
void
do_data(const object& sock, const char* buf, size_t size)
{
  string& tok(*sock.user<string>());
  tokenise(buf, buf + size, tok, '\n', echoline(sock));
}

@ No data has arrived for 15 seconds so the connection is shutdown (a FIN is
sent).  Any inflight data will still be delivered to the Service.

@<handle timer...@>=
void
do_rdexpire(const object& sock, unsigned& ms)
{
  shutdown(sock);
}

@ The |send()| function buffers the data to be written.

@<request...@>=
struct echoline {
  const object* const sock_;
  explicit
  echoline(const object& sock)
    : sock_(&sock)
  {
  }
  void
  operator ()(std::string& line)
  {
    @<prepare response@>@;
    send(*sock_, line.c_str(), line.size());
  }
};

@ Trim white-space, including any carriage-return characters, from the input.
Transform to upper-case.  Append CR/LF end-of-line sequence.  This is common
in many text-based protocols, such as \POP3/ and \SMTP/.

@<prepare...@>=
trim(line);
transform(line.begin(), line.end(), line.begin(), ucase);
line += "\r\n";

@* Build and Install.

The Makefile:

\yskip\.{CXXFLAGS = -Wall -Werror}

\.{LDFLAGS =}

\.{CXXMODULES = modsample}

\.{modsample\_OBJS = modsample.o}

\.{modsample\_LIBS = m}

\.{include augas.mk}

\yskip The configuration file:

\yskip\.{services = echo}

\.{service.echo.module = sample}

\.{service.echo.serv = 5000}

\.{module.sample.path = ./modsample}

\yskip You can send email to \pdfURL{the author}{mailto:mark@@emantic.co.uk} or visit
\pdfURL{the \AUG/ home page}{http://aug.sourceforge.net}.

@* Index.

Here is a list of the identifiers used, and where they appear.  Underlined
entries indicate the place of definition.  Error messages are also shown.
