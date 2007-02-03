from augas import *
from buffer import *
import log

# void writelog(level, msg)
# string error()
# void reconf()
# void stop()
# void post(sname, type, user)
# void delegate(sname, type, user)
# string getenv(name)
# void shutdown(sock)
# int tcpconnect(sname, host, serv, user)
# int tcplisten(sname, host, serv, user)
# void send(sock, buffer buf)
# void setrwtimer(sock, ms, flags)
# void resetrwtimer(sock, ms, flags)
# void cancelrwtimer(sock, flags)
# int settimer(sname, ms, user)
# bool resettimer(timer, ms)
# bool canceltimer(timer)

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
    log.debug("closed(): %s" % sock)

def teardown(sock):
    log.debug("teardown(): %s" % sock)
    for line in sock.user.lines(str(buf)):
        send(sock, sock.user.tail + "\n")
    shutdown(sock)

def accept(sock, addr, port):
    log.debug("accept(): %s" % sock)
    sock.user = Buffer()
    setrwtimer(sock, 5000, TIMRD)

def connected(sock, addr, port):
    log.debug("connected(): %s" % sock)

def data(sock, buf):
    log.debug("data(): %s" % sock)
    for line in sock.user.lines(str(buf)):
        send(sock, line + "\n")

def rdexpire(sock, ms):
    log.debug("rdexpire(): %s" % sock)
    send(sock, "are you there?\n")

def wrexpire(sock, ms):
    log.debug("wrexpire(): %s" % sock)

def expire(timer, ms):
    log.debug("expire(): %s" % timer)
