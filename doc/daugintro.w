\nocon % omit table of contents
\datethis % print date on listing

\def\AUG/{{\sc AUG}}
\def\AUGAS/{{\sc DAUG}}
\def\CYGWIN/{{\sc CYGWIN}}
\def\IPV6/{{\sc IPv6}}
\def\GNU/{{\sc GNU}}
\def\LINUX/{{\sc LINUX}}
\def\MINGW/{{\sc MINGW}}
\def\MSVC/{{\sc MSVC}}
\def\POP3/{{\sc POP3}}
\def\POSIX/{{\sc POSIX}}
\def\PYTHON/{{\sc PYTHON}}
\def\SMTP/{{\sc SMTP}}
\def\SMTP/{{\sc SMTP}}
\def\SSL/{{\sc SSL}}
\def\WINDOWS/{{\sc WINDOWS}}

@s std int @s string int

@s augas int
@s basic_factory int
@s basic_module int
@s basic_serv int
@s echoserv int
@s object int
@s serv_base int
@s user int

@f line normal

@* Introduction.
\AUGAS/ is an Open Source, Application Server written in \CEE/\AM\CPLUSPLUS/.
 It is part of the \pdfURL{\AUG/ project} {http://aug.sourceforge.net} which
is available for \LINUX/, \WINDOWS/ and other \POSIX/-compliant systems.
\AUGAS/ takes an unbiased view towards the systems it supports; it does not
favour one over another, and runs natively on all.  \AUGAS/ includes support
for:

\yskip
\item{$\bullet$} \IPV6/;
\item{$\bullet$} non-blocking IO;
\item{$\bullet$} \MINGW/ and \MSVC/;
\item{$\bullet$} plug-in modules;
\item{$\bullet$} \PYTHON/;
\item{$\bullet$} \SSL/.

\yskip\noindent
This document is a brief introduction to building and installing Modules for
the \AUGAS/ Application Server.  For further information, please visit the
\pdfURL{\AUG/ home page}{http://aug.sourceforge.net} or email myself,
\pdfURL{Mark Aylett}{mailto:mark@@emantic.co.uk}.

@* Event Model.

Threads are best suited to processing, not waiting.  Ideally, they should
focus on improving CPU utilisation on multi-processor machines.

\yskip\noinden
Threads are often used in combination with blocking APIs; secondary threads
allow processing to continue while blocking operations are in progress.
Unless secondary threads are managed with care, complexity, resource
contention and the risk of deadlocks will increase.

\yskip\noindent
Using non-blocking IO, a single thread can be dedicated to the de-multiplexing
of network events.  This event thread can be kept responsive by delegating
sizeable units of work to worker threads.  Worker threads can avoid the risk
of deadlocks by posting event notifications back to the event queue.

\yskip\noindent
The \AUGAS/ Application Server implements such an event model to de-multiplex
activity on signal, socket, timer and user-event objects.

@ \AUGAS/ propagates event notifications to Modules.  Modules are dynamically
loaded into the Application Server at run-time.  Each Module provides one or
more Services.  Modules and Services are wired together at configuration-time,
not compile-time.

\yskip\noindent
All Module calls are dispatched from the event thread (similar to a UI
thread).  A Service can either opt for a simple, single-threaded model, or a
suitable alternative, such as a thread-pool, depending on its requirements.

\yskip\noindent
The separation of physical Modules and logical Services allows Modules to
adapt and extend the host environment exposed to Services.  The \.{augpy}
Module, for example, exposes a \PYTHON/ module which encapsulates the \AUGAS/
host environment.  This allows Services to be implemented in \PYTHON/.

\yskip\noindent
Modules help to promote component, rather than source-level reuse.  Services
can interact by posting events to one another.  This allows Services to bridge
language boundaries.

\yskip\noindent
\AUGAS/ presents a uniform interface to system administrators across all
platforms.  Although, on \WINDOWS/, D\ae monised \AUGAS/ processes take the
form of NT services, from a sys-admin perspective, the interface remains the
same.  The following command can still be used to start the service from a
command window:

\yskip\noindent
\.{C:\\> daug -f daug.conf start}

@* Sample Module.
In the sections below, a Module is constructed in \CPLUSPLUS/ that:

\yskip
\item{$\bullet$} exposes a TCP service;
\item{$\bullet$} reads line-based input from clients;
\item{$\bullet$} echos lines back to clients in upper-case;
\item{$\bullet$} disconnects inactive clients.

\yskip\noindent
The Module is implemented in a single source file.  The layout of this file
is:

@c
@<include headers@>@;
namespace {@/
@<implement echo service@>@;
}@/
@<declare export table@>

@ The \.{<augaspp.hpp>} header is provided to aid Module implementations in
\CPLUSPLUS/.  Modules can also be written in \CEE/.  A \CEE/ implementation
would use the \.{<augas.h>} header.  For convenience, names are imported from
the |augas| and |std| namespaces.

@<include...@>=
#include <augaspp.hpp>@/
using namespace augas;@/
using namespace std;

@ The Service type, |echoserv| in this case, is fed into class templates which
simplify the \CEE/ to \CPLUSPLUS/ translation.  |basic_module<>| delegates the
task of creating services to a factory object whose type is specified by the
template argument.  |basic_factory<>| is used to create a simple factory for
the |echoserv| Service.

\yskip\noindent
\AUGAS/ Modules are required to export two library functions, namely
|augas_init()| and |augas_term()|.  The |AUGAS_MODULE| macro defines these two
export functions.

@<declare...@>=
typedef basic_module<basic_factory<echoserv> > sample;@/
AUGAS_MODULE(sample::init, sample::term)

@ \CPLUSPLUS/ Services implement the |serv_base| interface.  Stub
implementations to most of |serv_base|'s pure virtual functions are provided
by the |basic_serv| class.  For simplicity, |echoserv| is derived from
|basic_serv|.  The |echoline| functor handles each line received from the
client.

@<implement...@>=
@<echoline functor@>@;
struct echoserv : basic_serv {@/
  bool
  do_start(const char* sname);

  bool
  do_accept(object& sock, const char* addr, unsigned short port);

  void
  do_closed(const object& sock);

  void
  do_data(const object& sock, const void* buf, size_t size);

  void
  do_rdexpire(const object& sock, unsigned& ms);

  static serv_base*
  create(const char* sname);
};@/
@<member functions@>

@ The |do_start()| function is called to start the Service.  This is where
Service initialisation is performed.  In this case, a TCP listener is bound to
a port which is read from the configuration file using the |getenv()|
function.  If the ``service.echo.serv'' property is missing from the
configuration file, |false| is returned to prevent the Service from starting.

@<member...@>+=
bool
echoserv::do_start(const char* sname)
{
  writelog(AUGAS_LOGINFO, "starting service [%s]", sname);
  const char* port = augas::getenv("service.echo.serv");
  if (!port)
    return false;
  tcplisten("0.0.0.0", port);
  return true;
}

@ The |do_accept()| function is called when a new client connection is
accepted.  The |setuser()| function binds an opaque, user-defined value to an
\AUGAS/ object.  Here, a |string| buffer is assigned to track incomplete line
data received from the client.  An initial, {\sc ``HELLO''} message is sent to
the client.  The call to |setrwtimer()| establishes a timer that will expire
when there has been no read activity on the |sock| object for a period of 15
seconds or more.  \AUGAS/ will automatically reset the timer when read
activity occurs.

@<member...@>+=
bool
echoserv::do_accept(object& sock, const char* addr, unsigned short port)
{
  sock.setuser(new string());
  send(sock, "HELLO\r\n", 7);
  setrwtimer(sock, 15000, AUGAS_TIMRD);
  return true;
}

@ When a connection is closed, the |string| buffer associated with the socket
object is deleted.

@<member...@>+=
void
echoserv::do_closed(const object& sock)
{
  delete sock.user<string>();
}

@ |do_data()| is called whenever new data is received from a client.  The
|tok| reference is bound to the |string| buffer used to store incomplete lines
between calls to |do_data|.  The |tokenise()| function appends new data to the
back of |tok|.  Each complete line is processed by the |echoline| functor
before |tok| is cleared.

@<member...@>+=
void
echoserv::do_data(const object& sock, const void* buf, size_t size)
{
  string& tok(*sock.user<string>());
  tokenise(static_cast<const char*>(buf),
    static_cast<const char*>(buf) + size, tok, '\n', echoline(sock));
}

@ Read-timer expiry is communicated using the |do_rdexpire()| function.  If no
data arrives for 15 seconds, the connection is shutdown.  The |shutdown()|
function sends a FIN packet after ensuring that all buffered data has been
flushed.  \AUGAS/ ensures that any inflight messages sent by the client are
still delivered to the Service.

@<member...@>+=
void
echoserv::do_rdexpire(const object& sock, unsigned& ms)
{
  shutdown(sock);
}

@ The |basic_factory| template requires that Service classes define a static
|create()| function.

@<member...@>+=
serv_base*
echoserv::create(const char* sname)
{
  return 0 == strcmp(sname, "echo") ? new echoserv() : 0;
}

@ Each time the |echoline| functor is called, a response is prepared and sent
to the client associated with the |sock_| object.

@<echoline...@>=
struct echoline {
  object sock_;
  explicit
  echoline(const object& sock)
    : sock_(sock)
  {
  }
  void
  operator ()(std::string& line)
  {
    @<prepare response@>@;
    send(sock_, line.c_str(), line.size());
  }
};

@ White-space, including any carriage-returns, are trimmed from the input
line.  Then, the response is prepared by converting the line to upper-case and
appending a CR/LF pair.  This end-of-line sequence is common in text-based,
network protocols such as \POP3/ and \SMTP/.

@<prepare...@>=
trim(line);
transform(line.begin(), line.end(), line.begin(), ucase);
line += "\r\n";

@* Build and Install.
The \AUG/ package includes a \GNU/ makefile, named \.{aug.mk}, that can be
used to build Modules.  Simple Modules do not have any link-time dependencies;
all dependencies are injected into the Module when the Module is initialised.
Here is the complete \.{Makefile}:

\yskip
\.{CXXFLAGS = -I\$(AUG\_HOME)/include}

\.{CXXMODULES = modsample}

\.{modsample\_OBJS = modsample.o}

\.{include \$(AUG\_HOME)/etc/aug.mk}

@ First, the list of \CPLUSPLUS/ Modules to build are assigned to the
|CXXMODULES| variable:

\yskip
\.{CXXMODULES = modsample}

\yskip\noindent
Each Module can specify a list of |OBJS| and |LIBS|:

\yskip
\.{modsample\_OBJS = modsample.o}

\yskip\noindent
Finally, include the template makefile:

\yskip
\.{include \$(AUG\_HOME)/etc/aug.mk}

@ To configure the new Module, first add the name of the Service to the
``services'' property in the \.{daug.conf} file:

\yskip
\.{services = echo}

\yskip\noindent
Then, specify the Module that provides the Service, along with any additional
properties required by the Service implementation:

\yskip
\.{service.echo.module = sample}

\.{service.echo.serv = 5000}

\yskip\noindent
Finally, specify the path to the Module.  The file extension defaults to |dll|
on \WINDOWS/, and |so| otherwise:

\yskip
\.{module.sample.path = ./modsample}

@* Index.
