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
# bool resettimer (string sname, int tid, unsigned ms);
# bool canceltimer (string sname, int tid);

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    tcplisten(sname, "0.0.0.0", getenv("session.modskel.serv"), None)

def reconf(sname):
    log.debug("reconf(): %s" % sname)

def event(sname, type, user):
    log.debug("event(): %s" % sname)

def closed(sname, id, user):
    log.debug("closed(): %s" % sname)

def teardown(sname, cid, user):
    log.debug("teardown(): %s" % sname)
    for line in user.lines(str(buf)):
        send(cid, user.tail + "\n")
    shutdown(cid)

def accept(sname, cid, user, addr, port):
    log.debug("accept(): %s" % sname)
    setrwtimer(cid, 5000, TIMRD)
    return Buffer()

def connected(sname, cid, user, addr, port):
    log.debug("connected(): %s" % sname)

def data(sname, cid, user, buf):
    log.debug("data(): %s" % sname)
    for line in user.lines(str(buf)):
        send(cid, line + "\n")

def rdexpire(sname, cid, user, ms):
    log.debug("rdexpire(): %s" % sname)
    send(cid, "are you there?\n")

def wrexpire(sname, cid, user, ms):
    log.debug("wrexpire(): %s" % sname)

def expire(sname, tid, user, ms):
    log.debug("expire(): %s" % sname)
