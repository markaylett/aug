SHELL = /bin/sh
UNAME := $(shell uname | sed -e 's/CYGWIN.*/WIN32/i' -e 's/MINGW.*/WIN32/i')

.SUFFIXES:
.SUFFIXES: .c .cpp .exe .o

CC = gcc
RM = rm

ifeq ($(UNAME), WIN32)
AUG_HOME = c:/aug
PYTHON_HOME = c:/python25
DLLEXT = dll
EXEEXT = exe
COMMON =
CDEFS = \
	-DWINVER=0x0501
CFLAGS = \
	-mno-cygwin \
	-mthreads
LDFLAGS = \
	-mno-cygwin
LIBS =
else
AUG_HOME = $(HOME)
PYTHON_HOME = /usr
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
	-D_MT
CFLAGS += \
	-Wall \
	-Werror \
	-I$(AUG_HOME)/include \
	-I$(PYTHON_HOME)/include
LDFLAGS += \
	-L$(AUG_HOME)/lib \
	-L$(PYTHON_HOME)/libs
LIBS += \
	-lpython25

SRC = \
	modpython.c

OBJ = $(SRC:%.c=%.o)
NAME = modpython
SONAME = $(NAME).$(DLLEXT)

all: $(SONAME)

clean:
	$(RM) -f $(OBJ) $(SONAME)

$(SONAME): $(OBJ)
	$(CC) $(COMMON) -shared -Wl,-soname,$(SONAME) $(LDFLAGS) $(CDEFS) -o $@ $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(COMMON) $(CFLAGS) $(CDEFS) -c -o $@ $<

.PHONY: all clean
