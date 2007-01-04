from augas import *

def opensess(sname):
    tcplisten(sname, "0.0.0.0", getenv("session.modecho.serv"))

def data(sname, cid, user, buf):
    send(sname, cid, buf, SNDSELF)
