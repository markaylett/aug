import re

Quit = object()

class Interpreter:
    def __init__(self, cmds):
        self.cmds = cmds

    def __call__(self, line):
        line = line.strip()
        if len(line) == 0:
            return None
        toks = re.split(r"\s+", line.strip())
        try:
            x = apply(getattr(self.cmds, "do_" + toks[0].lower()), toks[1:])
            if x is not Quit:
                if x is None:
                    x = "+OK"
                elif hasattr(x, "__iter__"):
                    x = reduce(lambda y, z: y + str(z) + "\r\n", \
                               x, "+OK\r\n") + ".";
                else:
                    x = "+OK " + str(x)
        except AttributeError, e:
            x = "-ERR invalid command: %s" % e
        except TypeError, e:
            x = "-ERR invalid arguments: %s" % e
        except Exception, e:
            x = "-ERR exception: %s" % e
        return x
