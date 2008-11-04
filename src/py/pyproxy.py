from augpy import *
import log

pairs = {}

def start(sname):
    log.info("binding proxy listener")
    tcplisten("0.0.0.0", getenv("session.pyproxy.serv"), None, None)

def closed(sock):
    global pairs
    if pairs.has_key(sock):
        to = pairs[sock]
        log.info("closing proxy pair: %s -> %s" % (sock, to))
        del pairs[sock]
        del pairs[to]
        shutdown(to, 0)

def accepted(sock, name):
    global pairs
    log.info("opening proxy pair")
    # connected() will always be called after tcpconnect() returns.
    to = tcpconnect("localhost", getenv("session.pyproxy.to"), None, None)
    pairs[sock] = to
    pairs[to] = sock

def connected(sock, name):
    log.info("proxy pair established")

def recv(sock, buf):
    # It is safe to send to a connection that has not been fully established.
    if pairs.has_key(sock):
        send(pairs[sock], buf)
