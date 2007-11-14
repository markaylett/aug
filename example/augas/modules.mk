ifndef AUG_HOME
$(error please define AUG_HOME)
endif

CFLAGS = -I$(AUG_HOME)/include
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L$(AUG_HOME)/lib

CMODULES = modminimal
CXXMODULES = \
	modclient \
	modcommand \
	modhttp \
	modsched \
	modserver

modclient_OBJS = client.o
modclient_LIBS = augutil augsys

modcommand_OBJS = command.o
modcommand_LIBS = augutil augsys

modhttp_OBJS = http.o
modhttp_LIBS = augnet augutil augmar augsys

modminimal_OBJS = minimal.o

modsched_OBJS = sched.o
modsched_LIBS = augnet augutil augsys

modserver_OBJS = server.o

all: all-aug

clean: clean-aug

bench: all
	$(AUG_HOME)/bin/daug -f bench.conf test

test: all
	$(AUG_HOME)/bin/daug -f test.conf test

%.png: %.dat
	Rscript bench.R <$ $@

include $(AUG_HOME)/etc/aug.mk
