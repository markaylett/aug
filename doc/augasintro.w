\nocon % omit table of contents
\datethis % print date on listing

\def\AUG/{{\sc AUG}}
\def\AUGAS/{{\sc AUGAS}}
\def\CYGWIN/{{\sc CYGWIN}}
\def\GNU/{{\sc GNU}}
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
@s echoserv int
@s object int
@s serv_base int
@s user int

@f line normal

@* Introduction.
\AUGAS/ is an Open Source, Network Application Server written in
\CEE//\CPLUSPLUS/.  It is part of the \pdfURL{\AUG/ project}
{http://aug.sourceforge.net}, and has been ported to a number of OSes
including \LINUX/ and \WINDOWS/.  \AUGAS/ runs natively on \WINDOWS/; it does
not require a porting layer, such as \CYGWIN/, to run.  Dependencies are kept
to a minimum.

@ Threads are often forced upon an application by APIs that use a blocking
mode of operation.  The application must then introduce threads to effectively
bypass the blockage.  If these threads are allowed to reach into other areas
of the application, complexity, resource contention and the risk of deadlocks
will increase.

\bigskip\noindent
Threads are best suited to processing, not waiting.  Where possible, they
should be used to improve parallelisation on multi-processor machines.

\bigskip\noindent
Using non-blocking IO, a single thread can be dedicated to waiting for, and
de-multiplexing, network events.  The event thread can be kept responsive by
delegating sizeable units of work off to worker threads.  Worker threads can
avoid the risk of deadlocks by posting events back to the event queue.

\bigskip\noindent
The \AUGAS/ Application Server uses an efficient, event-based model to
de-multiplex activity on signal, socket, timer and user-event objects.

@ \AUGAS/ communicates event notifications to Modules.  Modules are
dynamically loaded into the the Application Server at run-time.  Each Module
provides one or more Services.  Modules and Services are wired together at
configuration-time, not compile-time.

\bigskip\noindent
All Module calls are dispatched from the event thread (similar to a UI
thread).  A Service can either opt for a simple, single-threaded model, or a
suitable alternative, such as a thread-pool, depending on its requirements.

\bigskip\noindent
The separation of physical Modules and logical Services allows Modules to
adapt and extend the host environment exposed to Services.  The \.{augpy}
Module, for example, exposes a \PYTHON/ module which encapsulates the \AUGAS/
host environment.  This allows Services to be implemented in \PYTHON/.

\bigskip\noindent
Modules help to promote component, rather than source-level reuse.  Services
can interact by posting events to one another.  This allows Services to bridge
language boundaries.

\bigskip\noindent
\AUGAS/ presents a uniform interface to system administrators across all
platforms.  Although, on \WINDOWS/, D\ae monised \AUGAS/ processes take the
form of NT services, from a sys-admin perspective, the interface remains the
same.  The following command can still be used to start the service from a
command window:

\yskip\noindent
\.{C:\\> augasd -f augasd.conf start}

@* Sample Module.
In the sections below, a Module is built in \CPLUSPLUS/ that:

\yskip
\item{$\bullet$} exposes a TCP service;
\item{$\bullet$} reads line-based input from clients;
\item{$\bullet$} echos lines back to clients in upper-case;
\item{$\bullet$} disconnects inactive clients.

\bigskip\noindent
The basic outline of the Module is:

@c
@<include headers@>@;
namespace {@/
@<implement service@>@;
}@/
@<declare export table@>

@ The \.{<augaspp.hpp>} header provides a set of utiliy functions and classes
designed to facilitate Module implementations in \CPLUSPLUS/.  Modules can,
however, just as easily be written in \CEE/.  A \CEE/ implementation would use
the \.{<augas.h>} header.  For convenience, names are imported from the
|augas| and |std| namespaces.

@<include...@>=
#include <augaspp.hpp>@/
using namespace augas;@/
using namespace std;

@ The |echoline| functor is defined to handle each line received from the
client.  \CPLUSPLUS/ Services implement the |serv_base| interface.  Stub
implementations to most of |serv_base|'s pure virtual functions are provided
by |basic_serv|.  For simplicity, |echoserv| is derived from |basic_serv|.

@<implement...@>=
@<echoline functor@>@;
struct echoserv : basic_serv {@/
@<start service@>@/
@<accept connection@>@/
@<cleanup on disconnect@>@/
@<process data@>@/
@<handle timer expiry@>@/
};

@ \AUGAS/ Modules are required to export two library functions, namely
|augas_init()| and |augas_term()|.

The |serv_base| derived type, |echoserv| in this case, is fed into class
templates which deal with the \CEE/ to \CPLUSPLUS/ translation.  The
|basic_factory| template is intended for use by Modules that provide a single
Service.  The |AUGAS_MODULE| macro defines the export functions.

@<declare...@>= typedef basic_module<basic_factory<echoserv> > sample;@/
AUGAS_MODULE(sample::init, sample::term)

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
line.  The response is then prepared by converting the line to upper-case and
appending a CR/LF pair.  This end-of-line sequence is commonly used by
text-based, network protocols such as \POP3/ and \SMTP/.

@<prepare...@>=
trim(line);
transform(line.begin(), line.end(), line.begin(), ucase);
line += "\r\n";

@ The |do_start()| function is called to start the Service.  This is where
Service initialisation is performed.  In this case, a TCP listener is bound to
a port which is read from the configuration file using the |getenv()|
function.  If the ``service.echo.serv'' property is missing from the
configuration file, |false| is returned to prevent the Service from starting.

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

@ The |setuser()| function binds an opaque, user-defined value to an \AUGAS/
object.  Here, A |string| buffer is assigned to track incomplete line data
received from the client.  An initial, {\sc ``HELLO''} message is sent to the
client.  The call to |setrwtimer()| establishes a timer that will expire when
there has been no read activity on the |sock| object for a period of 15
seconds or more.  \AUGAS/ will automatically reset the timer when read
activity occurs.

@<accept...@>=
bool
do_accept(object& sock, const char* addr, unsigned short port)
{
  sock.setuser(new string());
  send(sock, "HELLO\r\n", 7);
  setrwtimer(sock, 15000, AUGAS_TIMRD);
  return true;
}

@ Delete the |string| buffer associated with the socket object.

@<cleanup...@>=
void
do_closed(const object& sock)
{
  delete sock.user<string>();
}

@ The |tok| reference is bound to the |string| buffer used to store incomplete
lines between calls to |do_data|.  The |tokenise()| function appends new data
to the back of |tok|.  Each complete line is processed by the |echoline|
functor before |tok| is cleared.

@<process data...@>=
void
do_data(const object& sock, const char* buf, size_t size)
{
  string& tok(*sock.user<string>());
  tokenise(buf, buf + size, tok, '\n', echoline(sock));
}

@ If no data arrives for 15 seconds, the connection is shutdown.  The
|shutdown()| function sends a FIN packet after ensuring that all buffered data
has been flushed.  \AUGAS/ ensures that any inflight messages sent by the client
are still delivered to the Service.

@<handle timer...@>=
void
do_rdexpire(const object& sock, unsigned& ms)
{
  shutdown(sock);
}

@* Build and Install.
The \AUG/ package includes a \GNU/ makefile, named \.{aug.mk}, that can be
used to build Modules.  Simple Modules do not have any link-time dependencies;
all dependencies are injected into the Module when the Module is initialised.

@ First, assign a list of \CPLUSPLUS/ Modules to the |CXXMODULES| variable:

\yskip
\.{CXXMODULES = modsample}

\bigskip\noindent
Each Module can specify a list of |OBJS| and |LIBS|, as follows:

\yskip
\.{modsample\_OBJS = modsample.o}

\.{modsample\_LIBS = m}

\bigskip\noindent
Finally, include the template makefile:

\yskip
\.{include aug.mk}

@ To configure the new Module, first add the name of the Service to the
``services'' property in the \.{augasd.conf} file:

\yskip
\.{services = echo}

\bigskip\noindent
Then, specify the Module that provides the Service, along with any additional
properties required by the Service implementation:

\yskip
\.{service.echo.module = sample}

\.{service.echo.serv = 5000}

\bigskip\noindent
Finally, specify the path to the Module.  The file extension defaults to |dll|
on \WINDOWS/, and |so| otherwise:

\yskip
\.{module.sample.path = ./modsample}

@ For further information, please visit the \pdfURL{\AUG/ home
page}{http://aug.sourceforge.net} or email myself, \pdfURL{Mark
Aylett}{mailto:mark@@emantic.co.uk}.

@* Index.
