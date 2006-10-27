SHELL = /bin/sh
UNAME := $(shell uname | sed -e 's/CYGWIN.*/WIN32/i' -e 's/MINGW.*/WIN32/i')

.SUFFIXES:
.SUFFIXES: .c .cpp .exe .o

CC = g++
RM = rm

ifeq ($(UNAME), WIN32)
AUG_HOME = c:/aug
DLLEXT = dll
EXEEXT = exe
COMMON =
CDEFS = \
	-DWINVER=0x0501
CFLAGS = \
	-mno-cygwin \
	-mthreads
LDFLAGS =
LIBS =
else
AUG_HOME = $(HOME)
DLLEXT = so
EXEEXT =
COMMON =
CDEFS =
CFLAGS = \
	-fPIC \
	-DPIC
LDFLAGS =
LIBS =
endif

COMMON += \
	-ggdb
CDEFS += \
	-D_MT\
	-D_POSIX_SOURCE \
	-D_BSD_SOURCE
CFLAGS += \
	-Wall \
	-Werror \
	-pedantic \
	-I$(AUG_HOME)/include
LDFLAGS += \
	-L$(AUG_HOME)/lib
LIBS +=

SRC = \
	modskel.cpp

OBJ = $(SRC:%.cpp=%.o)
NAME = modskel
SONAME = $(NAME).$(DLLEXT)

all: $(SONAME)

clean:
	$(RM) -f $(OBJ) $(SONAME)

$(SONAME): $(OBJ)
	$(CC) $(COMMON) -shared -Wl,-soname,$(SONAME) $(LDFLAGS) $(CDEFS) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp
	$(CC) $(COMMON) $(CFLAGS) $(CDEFS) -c -o $@ $<

.PHONY: all clean
