# Makefile:
# NAME = modxxx
# SRC = \
# 	$(NAME).c
#
# include augas.mk

SHELL = /bin/sh
UNAME := $(shell uname | sed -e 's/CYGWIN.*/WIN32/i' -e 's/MINGW.*/WIN32/i')

.SUFFIXES:
.SUFFIXES: .c .dll .exe .so .o

CC = gcc
RM = rm

ifeq ($(UNAME), WIN32)
AUG_HOME = c:/aug
DLLEXT = dll
EXEEXT = exe
COMMON = \
	-mno-cygwin \
	-mthreads
CDEFS = \
	-DWINVER=0x0501
CFLAGS =
LDFLAGS =
LIBS =
else
AUG_HOME = $(HOME)
DLLEXT = so
EXEEXT =
COMMON =
CDEFS = \
	-DPIC
CFLAGS = \
	-fPIC
LDFLAGS =
LIBS = -lpthread
endif

COMMON += \
	-ggdb \
	-O3

CDEFS += \
	-D_MT \
	-D_REENTRANT

CFLAGS += \
	-MMD \
	-MP	\
	-Wall \
	-Werror \
	-I$(AUG_HOME)/include

LDFLAGS += \
	-L$(AUG_HOME)/lib

LIBS += \
	-lm

OBJ = $(SRC:%.c=%.o)
DEP = $(SRC:%.c=%.d)

SONAME = $(NAME).$(DLLEXT)

all: $(SONAME)

clean:
	$(RM) -f $(SONAME) $(OBJ) $(DEP)

-include $(DEP)

$(SONAME): $(OBJ)
	$(CC) $(COMMON) -shared -Wl,-soname,$(SONAME) $(LDFLAGS) $(CDEFS) -o $@ $(OBJ) $(LIBS)

%.o: %.c
	$(CC) $(COMMON) $(CFLAGS) $(CDEFS) -c -o $@ $<

.PHONY: all clean
