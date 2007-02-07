from augas import *
import log

pairs = {}

def init(sname):
    log.info("binding proxy listener")
    tcplisten(sname, "0.0.0.0", getenv("session.pyproxy.serv"), None)

def closed(sock):
    global pairs
    if pairs.has_key(sock):
        to = pairs[sock]
        log.info("closing proxy pair: %s -> %s" % (sock, to))
        del pairs[sock]
        del pairs[to]
        shutdown(to)

def accept(sock, addr, port):
    global pairs
    log.info("opening proxy pair")
    # connected() will always be called after tcpconnect() returns.
    to = tcpconnect(sock.sname, "localhost", getenv("session.pyproxy.to"), None)
    pairs[sock] = to
    pairs[to] = sock

def connected(sock, addr, port):
    log.info("proxy pair established")

def data(sock, buf):
    # It is safe to send to a connection that has not been fully established.
    if pairs.has_key(sock):
        send(pairs[sock], buf)
