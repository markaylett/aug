from augas import *
import log

def init(sname):
    settimer(sname, 0, 5000, None)

def expire(timer, ms):
    stop()
