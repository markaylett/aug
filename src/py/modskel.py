from augas import *
from buffer import *
import log

# void writelog (int level, string msg);
# string error (void);
# void reconf (void);
# void stop (void);
# void post (string sname, int type, object user);
# void delegate (string sname, int type, object user);
# string getenv (string name);
# void shutdown (int sid);
# int tcpconnect (string sname, string host, string serv, object user);
# int tcplisten (string sname, string host, string serv, object user);
# void send (int cid, buffer buf);
# void setrwtimer (int cid, unsigned ms, unsigned flags);
# void resetrwtimer (int cid, unsigned ms, unsigned flags);
# void cancelrwtimer (int cid, unsigned flags);
# int settimer (string sname, int tid, unsigned ms, object user);
# bool resettimer (int tid, unsigned ms);
# bool canceltimer (int tid);

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    tcplisten(sname, "0.0.0.0", getenv("session.modskel.serv"), None)

def reconf(sname):
    log.debug("reconf(): %s" % sname)

def event(sname, type, user):
    log.debug("event(): %s" % sname)

def closed(sock):
    log.debug("closed(): %s" % sock.sname)

def teardown(sock):
    log.debug("teardown(): %s" % sock.sname)
    for line in sock.user.lines(str(buf)):
        send(sock.id, sock.user.tail + "\n")
    shutdown(sock.id)

def accept(sock, addr, port):
    log.debug("accept(): %s" % sock.sname)
    sock.user = Buffer()
    setrwtimer(sock.id, 5000, TIMRD)

def connected(sock, addr, port):
    log.debug("connected(): %s" % sock.sname)

def data(sock, buf):
    log.debug("data(): %s" % sock.sname)
    for line in sock.user.lines(str(buf)):
        send(sock.id, line + "\n")

def rdexpire(sock, ms):
    log.debug("rdexpire(): %s" % sock.sname)
    send(cid, "are you there?\n")

def wrexpire(sock, ms):
    log.debug("wrexpire(): %s" % sock.sname)

def expire(timer, ms):
    log.debug("expire(): %s" % timer.sname)
