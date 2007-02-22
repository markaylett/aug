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

All too often, threads are forced upon an application by APIs that use a
blocking mode of operation.  Unless these threads are managed carefully, they
may increase resource contention and the risk of deadlocks.

Threads are best suited to processing, not waiting.  Where possible, they
should be used to maximise parallelisation on multi-processor machines.

Socket-based applications can opt for a non-blocking mode of operation.  In
this mode, a single thread can be dedicated to waiting and de-multiplexing
network events.  Sizeable chunks of CPU-intensive work can then be delegated
off to worker threads.

@ The \AUGAS/ Application Server uses such an event-based model to
de-multiplex activity on signal, socket, timer and custom event objects.

These event notifications are then communicated to Modules hosted by the
Application Server.

Modules are physical components that are dynamically loaded into the the
application server at run-time.  Each Module provides one or more Services.
Modules and Services are wired together at configuration-time.

This system allows features common to many Services to be implemented in
Modules.  The \.{augpy} Module is an example of this, it enables Services to
be implemented in the \PYTHON/ language.  The \.{augpy} Module exposes
\AUGAS/'s host environment to \PYTHON/ modules (not to be confused with an
\AUGAS/ Module) acting as Services.

Modules help to promote component rather than source-level reuse.  Services
can co-operate using custom events.  This allows Services to bridge language
boundaries.

All Module calls are dispatched from the event thread (similar to a UI
thread).  A Service either can opt for a simple, single-threaded model, or a
more elaborate model such as a thread-pool, depending on its requirements.

@ \AUGAS/ is an Open Source application written in \CEE//\CPLUSPLUS/. It is
part of the \pdfURL{\AUG/ project}{http://aug.sourceforge.net}.  \AUGAS/ runs
natively on a variety of OSes, including \LINUX/ and
\WINDOWS/.  On \WINDOWS/, \AUGAS/ does not require a porting layer, such as \CYGWIN/,
to operate.  And, its dependencies are minimal.

\AUGAS/ presents a uniform interface to system administrators across all
platforms.  Although, on \WINDOWS/, D\ae monised \AUGAS/ process take the form
of NT services, from a sys-admin perspective, the interface remains the same.
The following command can be used to start the service from a command window:

\yskip\.{C:\\> augasd -f augasd.conf start}

@* Sample Module.

In the sections below, a module is built in \CPLUSPLUS/ that:

\yskip\item{$\bullet$} exposes a TCP service
\item{$\bullet$} reads line-based input from clients
\item{$\bullet$} echos input lines in upper-case
\item{$\bullet$} times-out when a client is inactive

\yskip The basic outline of the Module follows:

@c
@<include headers@>@;
namespace {@/
@<implement service@>@;
}@/
@<declare export table@>

@ The \.{<augaspp.hpp>} provides a set of utiliy functions and classes
designed to facilitate module implementations in \CPLUSPLUS/.  Modules can,
however, just as easily be written in \CEE/.  A \CEE/ implementation would use
the \.{<augas.h>} header.

@<include...@>=
#include <augaspp.hpp>

@ Services implement the |serv_base| interface.  Stub implementations to most
of the pure virtual functions are provided by |basic_serv|.  For convenience,
|serv| is derived from |basic_serv|.

@<implement...@>=
using namespace augas;@/
using namespace std;@/
@<echoline functor@>@;
struct serv : basic_serv {@/
@<start service@>@/
@<accept connection@>@/
@<cleanup on disconnect@>@/
@<process data@>@/
@<handle timer expiry@>@/
};

@ Here is the main program.

@<declare...@>=
typedef basic_module<basic_factory<serv> > module;@/
AUGAS_MODULE(module::init, module::term)

@ The |send()| function buffers the data to be written.

@<echoline...@>=
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

@ TODO

@<start...@>=
bool
do_start(const char* sname)
{
  writelog(AUGAS_LOGINFO, "starting service [%s]", sname);
  const char* port = augas::getenv("service.echo.serv");
  if (!port)
    return false;
  tcplisten("0.0.0.0", port);
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

@* Build and Install.

The \.{Makefile}:

\.{CXXMODULES = modsample}

\.{modsample\_OBJS = modsample.o}

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
