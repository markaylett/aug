from augas import *
import log

def init(sname):
    tcplisten(sname, "0.0.0.0", getenv("session.modecho.serv"), None)

def data(sname, cid, user, buf):
    log.info("echoing data")
    send(sname, cid, buf, SNDPEER)
