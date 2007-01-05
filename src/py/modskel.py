from augas import *
from buffer import *
import log

# string error (void);
# void reconf (void);
# void writelog (int level, string msg);
# void post (string sname, int type, object user);
# string getenv (string name);
# int tcpconnect (string sname, string host, string serv);
# int tcplisten (string sname, string host, string serv);
# int settimer (string sname, int tid, unsigned ms, object user);
# bool resettimer (string sname, int tid, unsigned ms);
# bool canceltimer (string sname, int tid);
# void shutdown (int fid);
# void send (string sname, int cid, buffer buf);
# void setrwtimer (int cid, unsigned ms, unsigned flags);
# void resetrwtimer (int cid, unsigned ms, unsigned flags);
# void cancelrwtimer (int cid, unsigned flags);

def closesess(sname):
    log.debug("closesess(): %s" % sname)

def opensess(sname):
    log.debug("opensess(): %s" % sname)
    tcplisten(sname, "0.0.0.0", getenv("session.modskel.serv"))

def event(sname, type, user):
    log.debug("event(): %s" % sname)

def expire(sname, tid, user, ms):
    log.debug("expire(): %s" % sname)

def reconf(sname):
    log.debug("reconf(): %s" % sname)

def closeconn(sname, cid, user):
    log.debug("closeconn(): %s" % sname)

def openconn(sname, cid, addr, port):
    log.debug("openconn(): %s" % sname)
    setrwtimer(cid, 5000, TIMRD)
    return Buffer()

def notconn(sname, cid, user):
    log.debug("notconn(): %s" % sname)

def data(sname, cid, user, buf):
    log.debug("data(): %s" % sname)
    for line in user.lines(str(buf)):
        send(sname, cid, line + '\n', SNDSELF)

def rdexpire(sname, cid, user, ms):
    log.debug("rdexpire(): %s" % sname)
    send(sname, cid, 'are you there?\n', SNDSELF)

def wrexpire(sname, cid, user, ms):
    log.debug("wrexpire(): %s" % sname)

def teardown(sname, cid, user):
    log.debug("teardown(): %s" % sname)
    for line in user.lines(str(buf)):
        send(sname, cid, user.tail + '\n', SNDSELF)
    shutdown(cid)
