import augas

# void reconf (void);
# void writelog (int level, string msg);
# void post (string sname, int type, object user);
# string getenv (string sname, string name);
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

class Buffer:
    def __init__(self):
        self.tail = ''

    def lines(self, data):
        data = self.tail + data
        ls = data.split('\n')
        self.tail = ls.pop()
        for l in ls:
            yield l

def logdebug(s):
    augas.writelog(augas.LOGDEBUG, s)

def closesess(sname):
    logdebug("closesess(): %s" % sname)

def opensess(sname):
    augas.writelog(augas.LOGDEBUG, "opensess(): %s" % sname)
    augas.tcplisten(sname, "0.0.0.0", "8080")

def event(sname, type, user):
    logdebug("event(): %s" % sname)

def expire(sname, tid, user, ms):
    logdebug("expire(): %s" % sname)

def reconf(sname):
    logdebug("reconf(): %s" % sname)

def closeconn(sname, cid, user):
    logdebug("closeconn(): %s" % sname)

def openconn(sname, cid, addr, port):
    logdebug("openconn(): %s" % sname)
    augas.setrwtimer(cid, 5000, augas.TIMRD)
    return Buffer()

def notconn(sname, cid, user):
    logdebug("notconn(): %s" % sname)

def data(sname, cid, user, buf):
    logdebug("data(): %s" % sname)
    for line in user.lines(str(buf)):
        augas.send(sname, cid, line + '\n', augas.SNDSELF)

def rdexpire(sname, cid, user, ms):
    logdebug("rdexpire(): %s" % sname)
    augas.send(sname, cid, 'are you there?\n', augas.SNDSELF)

def wrexpire(sname, cid, user, ms):
    logdebug("wrexpire(): %s" % sname)

def teardown(sname, cid, user):
    logdebug("teardown(): %s" % sname)
    for line in user.lines(str(buf)):
        augas.send(sname, cid, user.tail + '\n', augas.SNDSELF)
    augas.shutdown(cid)
