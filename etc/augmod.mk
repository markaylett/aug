# Makefile:
# NAME = modxxx
# SRC = \
# 	$(NAME).cpp
#
# include augmod.mk

SHELL = /bin/sh
UNAME := $(shell uname | sed -e 's/CYGWIN.*/WIN32/i' -e 's/MINGW.*/WIN32/i')

.SUFFIXES:
.SUFFIXES: .cpp .dll .exe .so .o

CXX = g++
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
CXXFLAGS =
LDFLAGS =
LIBS =
else
AUG_HOME = $(HOME)
DLLEXT = so
EXEEXT =
COMMON =
CDEFS = \
	-DPIC
CXXFLAGS = \
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

CXXFLAGS += \
	-MMD \
	-MP	\
	-Wall \
	-Werror \
	-I$(AUG_HOME)/include

LDFLAGS += \
	-L$(AUG_HOME)/lib

LIBS += \
	-lm

OBJ = $(SRC:%.cpp=%.o)
DEP = $(SRC:%.cpp=%.d)

SONAME = $(NAME).$(DLLEXT)

all: $(SONAME)

clean:
	$(RM) -f $(SONAME) $(OBJ) $(DEP)

-include $(DEP)

$(SONAME): $(OBJ)
	$(CXX) $(COMMON) -shared -Wl,-soname,$(SONAME) $(LDFLAGS) $(CDEFS) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp
	$(CXX) $(COMMON) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

.PHONY: all clean
