% modtest: An example Augas module by Mark Aylett

\nocon % omit table of contents
\datethis % print date on listing
\def\SPARC{SPARC\-\kern.1em station}

@* Introduction.  Augas is a network application server.

It is designed to manage the complexities associated with implementing
portable and efficient network servers.

@ Services.  A single application server instance may host many services.

@ Modules.  Each service is managed by a module; each module may manage
several services.  Module are physical components.

Modules extend and adapt the host environment provided by the application
server.

As such, they are ideally suited for implementing features that run
horizonally, across many services.

@ Host.  The host environment exposes several class of object that are common
to network servers.

@ Logging.  TODO

@ Config.  TODO

@ Events.  TODO

@ Sockets.  TODO

@ Timers.  

@ Test section.  This is a test.

@c
@<include header files@>@/
@<use namespace names@>@/
@<implement service@>@/
@<declare export table@>

@ We must include the standard I/O definitions, since we want to send
formatted output to |stdout| and |stderr|.

@<include...@>=
#include "augaspp.hpp"

@ TODO

@<use namespace...@>=
using namespace augas;

@ Why two-phase construction - reentrant.

@<implement...@>=
namespace {@/
struct serv : basic_serv {
  bool
  do_start(const char* sname)
  {
    writelog(AUGAS_LOGINFO, "starting...");
    tcplisten(sname, "0.0.0.0", "5000");
    return true;
  }
  void
  do_data(const object& sock, const char* buf, size_t size)
  {
    send(sock, buf, size);
  }
};@/
}

@ Here is the main program.

@<declare...@>=
typedef basic_module<basic_factory<serv> > module;@/
AUGAS_MODULE(module::init, module::term)

@* Index.
Here is a list of the identifiers used, and where they appear. Underlined
entries indicate the place of definition. Error messages are also shown.
