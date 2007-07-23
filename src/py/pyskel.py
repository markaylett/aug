#!/usr/bin/env python
#import sys
from augrt import *
from augutil import *
import log

# void writelog(level, msg)
# string error()
# void reconfall()
# void stopall()
# void post(to, type, user)
# void dispatch(to, type, user)
# string getenv(name, def)
# string getsession()
# void shutdown(sock, flags)
# int tcpconnect(host, serv, user)
# int tcplisten(host, serv, user)
# void send(sock, buffer buf)
# void setrwtimer(sock, ms, flags)
# void resetrwtimer(sock, ms, flags)
# void cancelrwtimer(sock, flags)
# int settimer(ms, user)
# bool resettimer(timer, ms)
# bool canceltimer(timer)

class Handler:
    def __init__(self):
        self.props = {}

    def do_exit(self):
        return Quit

    def do_quit(self):
        return Quit

    def do_get(self, x):
        return self.props[x]

    def do_ls(self):
        return self.props.iteritems()

    def do_set(self, x, y):
        self.props[x] = y

    def do_unset(self, x):
        del self.props[x]

interp = Interpreter(Handler())

# for line in sys.stdin:
#      x = interp.interpret(line)
#      if x == Quit:
#          sys.exit()
#      elif x != None:
#          print x

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    tcplisten("0.0.0.0", getenv("session.pyskel.serv"), None)

def reconf():
    log.debug("reconf()")

def event(frm, type, user):
    log.debug("event()")

def closed(sock):
    log.info("closed(): %s" % sock)

def teardown(sock):
    log.debug("teardown(): %s" % sock)
    shutdown(sock, 0)

def accepted(sock, addr, port):
    log.info("accepted(): %s" % sock)
    sock.user = LineParser()
    setrwtimer(sock, 15000, TIMRD)
    send(sock, "+OK hello\r\n")

def connected(sock, addr, port):
    log.debug("connected(): %s" % sock)

def data(sock, buf):
    log.debug("data(): %s" % sock)
    global interp
    for line in sock.user.parse(str(buf)):
        x = interp.interpret(line)
        if x == Quit:
            send(sock, "+OK goodbye\r\n")
            shutdown(sock, 0)
        elif x != None:
            send(sock, x + "\r\n")

def rdexpire(sock, ms):
    log.debug("rdexpire(): %s" % sock)
    send(sock, "+OK hello\r\n")

def wrexpire(sock, ms):
    log.debug("wrexpire(): %s" % sock)

def expire(timer, ms):
    log.debug("expire(): %s" % timer)
