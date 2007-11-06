from augpy import *
import log

def start(sname):
    tcplisten("0.0.0.0", getenv("session.pyecho.serv"), None)

def data(sock, buf):
    send(sock, buf)
