.SUFFIXES:
.SUFFIXES: .c .cpp .dll .exe .so .o

SHELL = /bin/sh
UNAME := $(shell uname | sed -e 's/CYGWIN.*/WIN32/i' \
	-e 's/MINGW.*/WIN32/i')

CC = gcc
CXX = g++
RM = rm

ifeq ($(UNAME), WIN32)
DLLEXT = dll
EXEEXT = exe
COPTS = \
	-mno-cygwin \
	-mthreads
CDEFS = \
	-DWINVER=0x0501
else
DLLEXT = so
EXEEXT =
CDEFS = \
	-DPIC
COPTS = \
	-fPIC
endif

CDEFS += \
	-D_MT \
	-D_REENTRANT

COPTS += \
	-MMD \
	-MP

DLLS = $(CMODULES:%=%.$(DLLEXT)) $(CXXMODULES:%=%.$(DLLEXT))

.PHONY: all clean

all: $(DLLS)

clean:
	$(RM) -f $(DEPS) $(DLLS) $(OBJS)

define MODULE_template
DEPS += $$($(1)_DEPS)
OBJS += $$($(1)_OBJS)
$(1).$(DLLEXT): $$($(1)_OBJS)
	$(2) $(COPTS) $(3) -shared -Wl,-soname,$(1).$(DLLEXT) \
		$(LDFLAGS) $(CDEFS) -o $(1).$(DLLEXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%)
endef

$(foreach module,$(CMODULES),$(eval $(call \
	MODULE_template,$(module),$(CC), $(CFLAGS))))

$(foreach module,$(CXXMODULES),$(eval \
	$(call MODULE_template,$(module),$(CXX), $(CXXFLAGS))))

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

-include $(DEPS)
