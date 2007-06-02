\nocon % omit table of contents
\datethis % print date on listing

\def\AUG/{{\sc AUG}}
\def\AUGRT/{{\sc DAUG}}
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

@s augrt int
@s basic_factory int
@s basic_module int
@s basic_session int
@s echosession int
@s object int
@s session_base int
@s user int

@f line normal

@* Introduction.
\AUGRT/ is an Open Source, Application Server written in \CEE/\AM\CPLUSPLUS/.
 It is part of the \pdfURL{\AUG/ project} {http://aug.sourceforge.net} which
is available for \LINUX/, \WINDOWS/ and other \POSIX/-compliant systems.
\AUGRT/ takes an unbiased view towards the systems it supports; it does not
favour one over another, and runs natively on all.  \AUGRT/ includes support
for \IPV6/ and \SSL/.

\yskip\noindent
This document is a brief introduction to building and installing Modules for
the \AUGRT/ Application Server.  For further information, please visit the
\pdfURL{\AUG/ home page}{http://aug.sourceforge.net} or email myself,
\pdfURL{Mark Aylett}{mailto:mark@@emantic.co.uk}.

@* Event Model.

Ideally, threads should to used to maximise CPU utilisation on multi-processor
machines.  Threads are, however, often used in combination with blocking APIs;
secondary threads allow execution to continue while blocking operations are in
progress.  Unless these secondary threads are carefully managed, complexity,
resource contention and the risk of deadlocks may increase.

\yskip\noindent
Using non-blocking IO, a single thread can be dedicated to the de-multiplexing
of network events.  This event thread can be kept responsive by delegating
large units of work to processing threads.  In this case, these processing
threads can reduce the risk of deadlocks by minimising shared state, and
constraining communicatings with the main thread to the event queue.  This is
similar, in fact, to a UI event model.

\yskip\noindent
The \AUGRT/ Application Server implements such an event model to de-multiplex
activity on signal, socket, timer and user-event objects.

@ \AUGRT/ delegates event notifications to physical Modules and, in turn,
Sessions.  Modules are dynamically loaded into the Application Server at
run-time.  Each Module manages one or more Sessions.  Modules and Sessions are
wired together at configuration-time, not compile-time.

\yskip\noindent
All Module calls are dispatched from the event thread.  A Session can,
therefore, either opt to implement a simple, single-threaded model, or a
suitable alternative, such as a thread-pool, depending on its own
requirements.  Following the tenets of Open Source, Module implementors are
free from artificial constraints imposed upon them by the host environment.

\yskip\noindent
The separation of physical Modules and logical Sessions allows Modules to
adapt and extend the host environment as viewed by Sessions.  The \.{augpy}
Module, for example, adapts the host environment to allow Sessions to be
written in \PYTHON/.  These language bindings are introduced by the Module
without change to \AUGRT/.

\yskip\noindent
Modules help to promote component, rather than source-level reuse.  Sessions
can interact with one-another by posting events to one another.  This allows
Sessions to bridge language boundaries.

\yskip\noindent
\AUGRT/ presents a uniform interface to system administrators across all
platforms.  Although, on \WINDOWS/, d\ae monised \AUGRT/ processes take the
form of NT services, from a sys-admin perspective, the interface remains the
same.  The following command can still be used to start the service from a
command prompt:

\yskip\noindent
\.{C:\\> daug -f daug.conf start}

@* Sample Module.
In the sections below, a Module is constructed, in \CPLUSPLUS/, that:

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
@<implement echo session@>@;
}@/
@<declare export table@>

@ The \.{<augrtpp.hpp>} header is provided to aid Module implementations in
\CPLUSPLUS/.  Modules can also be written in \CEE/;  \CEE/ implementations
would use the \.{<augrt.h>} header.  For convenience, names are imported from
the |augrt| and |std| namespaces.

@<include...@>=
#include <augrtpp.hpp>@/
using namespace augrt;@/
using namespace std;

@ The Session type - |echosession| in this case - is fed into class templates
which simplify the \CEE/ to \CPLUSPLUS/ translation.  |basic_module<>|
delegates the task of creating Sessions to a factory object, whose type is
specified by the template argument.  |basic_factory<>| is used to create a
simple factory for the |echosession| Session.

\yskip\noindent
\AUGRT/ Modules are required to export two library functions, namely
|augrt_init()| and |augrt_term()|.  The |AUGRT_MODULE| macro defines these two
export functions.

@<declare...@>=
typedef basic_module<basic_factory<echosession> > sample;@/
AUGRT_MODULE(sample::init, sample::term)

@ \CPLUSPLUS/ Sessions implement the |session_base| interface.  Stub
implementations for most of |session_base|'s pure virtual functions are
provided by the |basic_session| class.  For simplicity, |echosession| is
derived from |basic_session|.  The |echoline| functor handles each line
received from the client.

@<implement...@>=
@<echoline functor@>@;
struct echosession : basic_session {@/
  bool
  do_start(const char* sname);

  bool
  do_accepted(object& sock, const char* addr, unsigned short port);

  void
  do_closed(const object& sock);

  void
  do_data(const object& sock, const void* buf, size_t size);

  void
  do_rdexpire(const object& sock, unsigned& ms);

  static session_base*
  create(const char* sname);
};@/
@<member functions@>

@ The |do_start()| function is called to start the Session.  This is where
Session initialisation is performed.  In this case, a TCP listener is bound to
a port which is read from the configuration file using the |getenv()|
function.  If the ``session.echo.serv'' property is missing from the
configuration file, |false| is returned to deactivate the Session.

@<member...@>+=
bool
echosession::do_start(const char* sname)
{
  writelog(AUGRT_LOGINFO, "starting session [%s]", sname);
  const char* serv = augrt::getenv("session.echo.serv");
  if (!serv)
    return false;
  tcplisten("0.0.0.0", serv);
  return true;
}

@ The |do_accepted()| function is called when a new client connection is
accepted.  The |setuser()| function binds an opaque, user-defined value to an
\AUGRT/ object.  Here, a |string| buffer is assigned to track partial line
data received from the client.  An initial, {\sc ``HELLO''} message is sent to
the client.  The call to |setrwtimer()| establishes a timer that will expire
when there has been no read activity on the |sock| object for a period of 15
seconds or more.  \AUGRT/ will automatically reset the timer when read
activity occurs.

@<member...@>+=
bool
echosession::do_accepted(object& sock, const char* addr, unsigned short port)
{
  sock.setuser(new string());
  send(sock, "HELLO\r\n", 7);
  setrwtimer(sock, 15000, AUGRT_TIMRD);
  return true;
}

@ When a connection is closed, the |string| buffer associated with the socket
object is deleted.

@<member...@>+=
void
echosession::do_closed(const object& sock)
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
echosession::do_data(const object& sock, const void* buf, size_t size)
{
  string& tok(*sock.user<string>());
  tokenise(static_cast<const char*>(buf),
    static_cast<const char*>(buf) + size, tok, '\n', echoline(sock));
}

@ Read-timer expiry is communicated using the |do_rdexpire()| function.  If no
data arrives for 15 seconds, the connection is shutdown.  The |shutdown()|
function sends a FIN packet after ensuring that all buffered data has been
flushed.  \AUGRT/ ensures that any buffered messages are flushed before
performing the shutdown, and that any inflight messages sent by the client are
delivered to the Session.

@<member...@>+=
void
echosession::do_rdexpire(const object& sock, unsigned& ms)
{
  shutdown(sock);
}

@ The |basic_factory| template requires that Session classes define a static
|create()| function.

@<member...@>+=
session_base*
echosession::create(const char* sname)
{
  return 0 == strcmp(sname, "echo") ? new echosession() : 0;
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

@ To configure the new Module, first add the name of the Session to the
``sessions'' property in the \.{daug.conf} file:

\yskip
\.{sessions = echo}

\yskip\noindent
Then, specify the Module that provides the Session, along with any additional
properties required by the Session implementation:

\yskip
\.{session.echo.module = sample}

\.{session.echo.serv = 5000}

\yskip\noindent
Finally, specify the path to the Module.  The file extension defaults to |dll|
on \WINDOWS/, and |so| otherwise:

\yskip
\.{module.sample.path = ./modsample}

@* Index.
