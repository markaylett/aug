ifndef AUG_HOME
$(error please define AUG_HOME)
endif

CFLAGS = -I$(AUG_HOME)/include
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L$(AUG_HOME)/lib

PLATFORM = MINGW

CMODULES = modminimal
CXXMODULES = \
	modclient \
	modcommand \
	modsched \
	modserver

modclient_OBJS = client.o
modclient_LIBS = augutil augsys augctx

modcommand_OBJS = command.o
modcommand_LIBS = augutil augsys augctx

modminimal_OBJS = minimal.o

modsched_OBJS = sched.o
modsched_LIBS = augnet augutil augmar augsys augctx

modserver_OBJS = server.o
modserver_LIBS = augctx

all: all-base

clean: clean-base

bench: all
	$(AUG_HOME)/bin/augd -f bench.conf test

gprof:
	gprof $(AUG_HOME)/bin/augd gmon.out
#	| ./gprof2dot.py | dot -Tpng -o gmon.png

test: all
	$(AUG_HOME)/bin/augd -f test.conf test

%.png: %.dat
	Rscript bench.R <$ $@

include $(AUG_HOME)/etc/aug.mk
