\nocon % omit table of contents
\datethis % print date on listing

\def\ABI/{{\sc ABI}}
\def\AUG/{{\sc AUG}}
\def\CRLF/{{\sc CRLF}}
\def\CYGWIN/{{\sc CYGWIN}}
\def\FIN/{{\sc FIN}}
\def\IPV6/{{\sc IPv6}}
\def\GNU/{{\sc GNU}}
\def\LINUX/{{\sc LINUX}}
\def\MINGW/{{\sc MINGW}}
\def\MSVC/{{\sc MSVC}}
\def\POP3/{{\sc POP3}}
\def\POSIX/{{\sc POSIX}}
\def\PYTHON/{{\sc PYTHON}}
\def\RUBY/{{\sc RUBY}}
\def\SMTP/{{\sc SMTP}}
\def\SMTP/{{\sc SMTP}}
\def\SSL/{{\sc SSL}}
\def\WINDOWS/{{\sc WINDOWS}}
\def\XML/{{\sc XML}}

@s aug int
@s aug_boxptr int
@s basic_factory int
@s basic_module int
@s basic_session int
@s boxptr int
@s boxptr_base int
@s boxptrptr int
@s eachline int
@s echo int
@s error normal
@s mod int
@s mod_bool int
@s mod_handle int
@s mod_id int
@s object int
@s object_cast make_pair
@s objectref int
@s sessionptr int
@s static_cast make_pair
@s std int
@s string int
@s token int

@f line normal

@* Introduction.

\AUG/ is an event-driven network application-server, licensed under the GNU
General Public License (GPL).  \AUG/'s Module system makes it ideally suited
to building heterogeneous Internet applications.  Python and Ruby Modules are
bundled with the \AUG/ package.  The core system is written in portable
\CEE/\AM\CPLUSPLUS/.  \AUG/ also supports \IPV6/ and \SSL/, and is available
for \LINUX/, \WINDOWS/ and other \POSIX/-compliant systems.

\yskip\noindent

This document offers a introduction to building and installing \AUG/ Modules,
along with a brief insight into the application-server itself.  For further
information, please visit the \pdfURL{\AUG/ project home
page}{http://www.xofy.org/aug} or email me, \pdfURL{Mark
Aylett}{mailto:mark.aylett@@gmail.com}.

@* Event Model.

Carefully designed threading models can improve CPU utilisation on
multi-processor machines.  Similar effects, however, are rarely acheived when
ad hoc threads are used merely to ``simplify'' coding, or to enable
concurrency around blocking calls.  In such cases, complexity, resource
contention and the risk of deadlocks may actually increase, and performance
degrade.

\yskip\noindent

Non-blocking APIs that support event multiplexing reduce the need for many
threads.  Fewer context switches, locks, and fuller cache pipelines lead to
greater efficiencies.  The downside is that multiplexing code often results in
more complex state transitions.  In a sense, these transitions are the
flattened interleavings of the multi-threaded model.

\yskip\noindent

Multiplexing code is best confined to specialised components dedicated to such
purposes.  This is where \AUG/ comes in: the \AUG/ application-server uses an
event model, similar to the one described above, to multiplex signal, socket,
timer and custom-event activity.  Complexity is confined to the
application-server's internals.  Modules interact with the application-server
through a sanitised interface.

\yskip\noindent

\AUG/ conducts all multiplexing activity on its event thread.  The event
thread is kept responsive by delegating CPU-intensive tasks to worker threads.
Iteractions between the event thread and worker threads are confined to event
queues, which minimise the possibility of deadlocks.  This model is synonymous
with the UI event thread model.

@* Modules and Sessions.

\AUG/ delegates event notifications to Modules and, in turn, Sessions.
Modules are physical components, dynamically loaded into the
application-server at run-time.  Each Module manages one or more Sessions.
Modules and Sessions are wired together at configuration-time, not
compile-time.

\yskip\noindent

All Module calls are invoked from the event thread.  A Session can, therefore,
either opt for a simple, single-threaded model (by default), or a suitable
alternative such as a thread-pool --- \AUG/'s flexible design imposes fewer
constraints on Module authors.

\yskip\noindent

The separation of physical Modules and logical Sessions allows Modules to
adapt and extend the application-server environment exposed to Sessions.  The
\.{augpy} and \.{augrb} Modules, for example, adapt the host environment to
offer a \PYTHON/ or \RUBY/ oriented view to their associated Sessions.  These
language bindings are provided by the Modules, and are unbeknown to the
application-server.  A HTTP Session written in \RUBY/, for example, would be
managed by the \.{augrb} Module.

\yskip\noindent

Modules also help to promote component, rather than source-level reuse:
Sessions can interact with one-another by posting events to the event queue,
allowing Sessions to bridge language boundaries.

@* Objects.

@ \AUG/ defines an Application Binary Interface (\ABI/) for passing reference
counted Objects between Sessions.  These Objects may be authored in either
\CEE/ or \CPLUSPLUS/, use different compilers, or even different versions of
the same compiler.

\yskip\noindent

Interfaces are defined using a simple \XML/ schema.  It is often useful, for
example, to box a plain pointer into an Object so that it can be passed
between Sessions.  This allows the pointee's lifetime to be managed
automatically by the Object.  A example interface definition for a |boxptr|
follows.

\yskip
\.{<package name="aug" dir="augext">}

\.{  <interface name="boxptr">}

\.{    <method name="unboxptr" type="void*" qname="unboxptr"/>}

\.{  </interface>}

\.{</package>}

\yskip\noindent

Interfaces can be translated to a suitable ABI definition using the \.{augidl}
script.

\yskip\noindent

\.{C:\\> augidl -n boxptr <boxptr.xml >boxptr.h}

@* Administration.

System administrators are presented with a uniform interface across all
platforms.  On \WINDOWS/, a d\ae monised \AUG/ process takes the form of an NT
service, but from a sys-admin perspective, the command interface is the same
as for any other platform.  As with \LINUX/, the following command can be used
to start the service from a command prompt:

\yskip\noindent

\.{C:\\> augd -f augd.conf start}

@* Sample Module.

In the sections below, a Module is constructed in \CPLUSPLUS/ that:

\yskip
\item{$\bullet$} exposes a TCP service;
\item{$\bullet$} reads line-based input from clients;
\item{$\bullet$} echos the lines back to clients in upper-case;
\item{$\bullet$} disconnects inactive clients.

\yskip\noindent

The Module is implemented in a single source file.  The layout of this file
is:

@c
@<include headers@>@;
@<using declarations@>@;
namespace {@/
@<token object@>@;
@<eachline functor@>@;
@<echo session@>@;
}@/
@<declare export table@>

@ The \.{<augmodpp.hpp>} header is provided to simplify \CPLUSPLUS/ Module
implementations.  Modules can also be written in standard \CEE/.  A \CEE/
implementation would use the \.{<augmod.h>} header instead.

@<include...@>+=
#define MOD_BUILD
#include <augmodpp.hpp>@/

@ For convenience, names are imported from the |aug|, |mod| and |std|
namespaces.

@<using...@>+=
using namespace aug;@/
using namespace mod;@/
using namespace std;

@ The |boxptr| generated code, described earlier in this document, is included
in the module.

@<include...@>+=
#include <augext/boxptr.h>@/

@ Using the facilities in the |boxptr| header file, an implementation is
defined to box a |string|.  The |unboxptr_()| member function provides the
implementation behind the |unboxptr()| method.

@<token...@>+=
  class token : public boxptr_base<token> {@/
    string impl_;
  public:
    ~token()
    {
    }
    void*
    unboxptr_()
    {
      return &impl_;
    }
    static boxptrptr
    create()
    {
      return attach(new token());
    }
  };

@ A convenience function is defined to downcast from a base Object, and return
the ``unboxed'' pointer.

@<token...@>+=
string*
unboxtoken(objectref ob)
{
  boxptrptr box = object_cast<aug_boxptr>(ob);
  return null == box ? 0 : static_cast<string*>(unboxptr(box));
}

@ A call to the |eachline| functor prepares a response and sends it back to
the originating client.

@<eachline...@>=
  struct eachline {@/
    const mod_handle& sock_;
    explicit
    eachline(const mod_handle& sock)
      : sock_(sock)
    {
    }
    void
    operator ()(string& tok)
    {
      @<prepare response@>@;
      send(sock_, tok.c_str(), tok.size());
    }
  };

@ The \AUG/ libraries provide some standard string algorithms.

@<include...@>+=
#include <augutilpp/string.hpp>

@ White-space, including any carriage-returns, are trimmed from the input
line.  The response is then prepared by converting the line to upper-case and
appending a \CRLF/ pair.  This end-of-line sequence is chosen because of its
ubiquitous use in Internet protocols such as \POP3/ and \SMTP/.

@<prepare...@>=
trim(tok);
transform(tok.begin(), tok.end(), tok.begin(), ucase);
tok += "\r\n";

@ \AUG/ Modules are required to export three library functions: |mod_init()|,
|mod_term()| and |mod_create()|.  The |MOD_ENTRYPOINTS| macro assists with the
definition of these export functions.

\yskip\noindent

Class templates are used to bind the Module implementation to the library's
export table.  In this example, the Module is configured with a single Session
type, |echo|.  |basic_module<>| delegates the task of creating Sessions to a
factory type.  |basic_factory<>| builds a factory capable of creating |echo|
Sessions.

@<declare...@>=
typedef basic_module<basic_factory<echo> > module;@/
MOD_ENTRYPOINTS(module::init, module::term, module::create)

@* Echo Session.

\CPLUSPLUS/ Sessions are normally derived from the |basic_session| template.

@ Session functions of type |mod_bool| return either |MOD_TRUE| or
|MOD_FALSE|.  For those functions associated with a connection, a |MOD_FALSE|
return will result in the connection being closed.

@<echo...@>=
  class echo : public basic_session<echo> {@/
    const string sname_;
    explicit
    echo(const string& sname)
      : sname_(sname)
    {
    }
  public:
    ~echo()
    {
    }
    mod_bool
    start();

    void
    stop();

    void
    reconf();

    void
    event(const char* from, const char* type, mod_id id, objectref ob);

    void
    closed(mod_handle& sock);

    void
    teardown(mod_handle& sock);

    mod_bool
    accepted(mod_handle& sock, const char* name);

    void
    connected(mod_handle& sock, const char* name);

    mod_bool
    auth(mod_handle& sock, const char* subject, const char* issuer);

    void
    recv(mod_handle& sock, const void* buf, size_t len);

    void
    error(mod_handle& sock, const char* desc);

    void
    rdexpire(mod_handle& sock, unsigned& ms);

    void
    wrexpire(mod_handle& sock, unsigned& ms);

    void
    expire(mod_handle& timer, unsigned& ms);

    static sessionptr
    create(const char* sname);
  };@/
@<member functions@>

@ The |start()| function is called to start the Session.  This is where any
non-trivial resource acquisition and construction takes place.  The two-phase
construction is required to allow callbacks during the |start()| call.

\yskip\noindent

In this example, a TCP listener is bound to a port which is read from the
configuration file using the |mod::getenv()| function.  If the
``session.echo.serv'' property is missing from both the configuration file and
environment table, |MOD_FALSE| is returned and the Session deactivated.  The
simplicity of \SSL/ Sessions is also demonstrated.

@<member...@>+=
mod_bool
echo::start()
{
  writelog(MOD_LOGINFO, "starting...");
  const char* serv(mod::getenv("session.echo.serv"));
  const char* sslctx(mod::getenv("session.echo.sslcontext"));
  if (!serv)
    return MOD_FALSE;
  if (sslctx)
    writelog(MOD_LOGINFO, "sslcontext: %s", sslctx);
  tcplisten("0.0.0.0", serv, sslctx);
  return MOD_TRUE;
}

@ The |stop()| function is only called for a session whose |start()| function
returned |MOD_TRUE|.  There is no need to explicity close the listener socket,
as this will be done prior to |stop()| being call.

@<member...@>+=
void
echo::stop()
{
}

@ The |reconf()| handler is intended for Session's that wish to implement a
|SIGHUP|-style reconfiguration.  The function is actually called in response
to either a |SIGHUP| interrupt, or an explicit call to |reconfall()|.  On
Windows, |SIGHUP| equivalents are sent to the application-server via the
Service Manager.

@<member...@>+=
void
echo::reconf()
{
}

@ As mentioned earlier in this document, custom events can be passed between
Sessions.  The |event()| function is called when a custom event is delivered
to a Session.

@<member...@>+=
void
echo::event(const char* from, const char* type, mod_id id, objectref ob)
{
}

@ When a connection is closed, the |token| reference associated with the
|sock| handle is released.

@<member...@>+=
void
echo::closed(mod_handle& sock)
{
  aug_assign(sock.ob_, 0);
}

@ The |teardown()| function is called when the application-server is
terminating.  This function allows an application-level shutdown to take place
before any connections are forcibly closed.  In this case, the socket should
is simply shutdown.

@<member...@>+=
void
echo::teardown(mod_handle& sock)
{
  mod::shutdown(sock, 0);
}

@ The |accepted()| function is called when a new connection is established on
the listener socket.  A new |token| object is bound to the socket.  The
|token| object is user to track partial line data received from the client.
An initial, {\sc ``HELLO''} message is sent to the client.  The call to
|setrwtimer()| establishes a timer that will expire when no read activity has
occurred on the |sock| handle for a period of 15 seconds --- \AUG/ will
automatically reset the timer whenever read activity occurs.

@<member...@>+=
mod_bool
echo::accepted(mod_handle& sock, const char* name)
{
  boxptrptr bp = token::create();
  aug_assign(sock.ob_, bp.base());

  send(sock, "HELLO\r\n", 7);
  setrwtimer(sock, 15000, MOD_TIMRD);
  return MOD_TRUE;
}

@ This function is called when a connection, initiated by a call to
|tcpconnect()|, becomes established.

@<member...@>+=
void
echo::connected(mod_handle& sock, const char* name)
{
}

@ When \SSL/ sockets are used, authorisation of the peer certificate can be
conducted by the Session.

@<member...@>+=
mod_bool
echo::auth(mod_handle& sock, const char* subject, const char* issuer)
{
  return MOD_TRUE;
}

@ |recv()| is called when new data is received on a socket.  The Object
reference contains the |token| buffer used to store incomplete lines between
|recv()| calls.  The |tokenise()| function appends new data to the back of
|tok|.  Each complete line is processed by the |eachline| functor before |tok|
is cleared.

@<member...@>+=
void
echo::recv(mod_handle& sock, const void* buf, size_t len)
{
  string* tok = unboxtoken(sock.ob_);
  tokenise(static_cast<const char*>(buf),
    static_cast<const char*>(buf) + len, *tok, '\n',
    eachline(sock));
}

@ Socket-level errors are communicated to the Session via the |error()| call.

@<member...@>+=
void
echo::error(mod_handle& sock, const char* desc)
{
  writelog(MOD_LOGERROR, "server error: %s", desc);
}

@ Read-timer expiry is communicated using the |rdexpire()| function.  If no
data arrives for 15 seconds, the connection is shutdown.  The |shutdown()|
function sends a \FIN/ packet after ensuring that all buffered data has been
flushed.  \AUG/ ensures that any buffered messages are flushed before
performing the shutdown, and that any inflight messages sent by the client are
delivered to the Session.

@<member...@>+=
void
echo::rdexpire(mod_handle& sock, unsigned& ms)
{
  writelog(MOD_LOGINFO, "no data received for 15 seconds");
  shutdown(sock, 0);
}

@ Write-timers can also be scheduled, in which case, expiry would be notified
using the |wrexpire()| function.

@<member...@>+=
void
echo::wrexpire(mod_handle& sock, unsigned& ms)
{
}

@ In addition to read and write timers, abitrary timers can also be set using
|settimer()|.

@<member...@>+=
void
echo::expire(mod_handle& timer, unsigned& ms)
{
}

@ The |basic_factory| template requires that Session classes define a static
|create()| function.

@<member...@>+=
sessionptr
echo::create(const char* sname)
{
  return attach(new echo(sname));
}

@* Build and Install.

The \AUG/ package includes a \GNU/ makefile, named \.{aug.mk}, that can be
used to build Modules.  Simple Modules do not have any link-time dependencies;
all dependencies are injected into the Module when the Module is initialised.
Here is the complete \.{Makefile}:

\yskip
\.{CXXFLAGS = -I\$(AUG\_HOME)/include}

\.{CXXMODULES = modsample}

\.{modsample\_OBJS = modsample.o}

\.{all: all-aug}

\.{clean: clean-aug}

\.{include \$(AUG\_HOME)/etc/aug.mk}

@ First, the list of \CPLUSPLUS/ Modules to build are assigned to the
|CXXMODULES| variable:

\yskip
\.{CXXMODULES = modsample}

\yskip\noindent

Each Module can specify a list of |OBJS|, |LDFLAGS| and |LIBS|:

\yskip
\.{modsample\_OBJS = modsample.o}

\yskip\noindent

Finally, include the template makefile:

\yskip
\.{all: all-aug}

\.{clean: clean-aug}

\.{include \$(AUG\_HOME)/etc/aug.mk}

@ To configure the new Module, first add the name of the Session to the
``sessions'' property in the \.{augd.conf} file:

\yskip
\.{sessions = echo}

\yskip\noindent

Then, specify the Module that provides the Session, along with any additional
properties required by the Session implementation:

\yskip
\.{session.echo.module = sample}

\.{session.echo.serv = 5000}

\yskip\noindent

Finally, specify the path to the Module.  The file extension defaults to
|.dll| on \WINDOWS/, and |.so| otherwise:

\yskip
\.{module.sample.path = ./modsample}

\yskip
\yskip
\yskip
\yskip\noindent
Enjoy!

@* Index.
