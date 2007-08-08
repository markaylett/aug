.SUFFIXES:
.SUFFIXES: .c .cpp .dll .exe .so .o

SHELL = /bin/sh
UNAME := $(shell uname | sed -e 's/CYGWIN.*/WIN32/i' \
	-e 's/MINGW.*/WIN32/i')

AR = ar
CC = gcc
CXX = g++
RM = rm

ifeq ($(UNAME), WIN32)

COPTS = \
	-mno-cygwin \
	-mthreads
CDEFS = \
	-DWINVER=0x0501 \
	-DFD_SETSIZE=256

DLL_EXT = .dll
DLL_LDFLAGS = \
	-L. \
	-L/usr/lib/mingw \
	-Wl,--enable-auto-image-base
DLL_CLIBS =
DLL_CXXLIBS = \
	-nostdlib \
	-ldllcrt2 \
	-lstdc++ \
	-lgcc \
	-lmingw32 \
	-lmoldname \
	-lmingwex \
	-lmsvcrt \
	-luser32 \
	-lkernel32 \
	-ladvapi32 \
	-lshell32
DLL_CXXCRT = \
	libdllcrt2.a

EXE_EXT = .exe
EXE_LDFLAGS =
EXE_CLIBS =
EXE_CXXLIBS =

else

COPTS = \
	-fPIC
CDEFS = \
	-DPIC

DLL_EXT = .so
DLL_LDFLAGS =
DLL_CLIBS =
DLL_CXXLIBS =
DLL_CXXCRT =

EXE_EXT =
EXE_LDFLAGS =
EXE_CLIBS =
EXE_CXXLIBS =

endif

CDEFS += \
	-D_MT \
	-D_REENTRANT

COPTS += \
	-MMD \
	-MP

BINS = \
	$(CLIBRARIES:%=%$(DLL_EXT)) \
	$(CXXLIBRARIES:%=%$(DLL_EXT)) \
	$(CMODULES:%=%$(DLL_EXT)) \
	$(CXXMODULES:%=%$(DLL_EXT)) \
	$(CPROGRAMS:%=%$(EXE_EXT)) \
	$(CXXPROGRAMS:%=%$(EXE_EXT))

.PHONY: all clean

all: $(BINS)

clean:
	$(RM) -f $(DEPS) $(BINS) $(DLL_CXXCRT) $(OBJS) *~

define CMODULE_template
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(DLL_EXT): $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -shared -Wl,-soname,$(1)$(DLL_EXT) \
		$(LDFLAGS) $(DLL_LDFLAGS) -o $(1)$(DLL_EXT) $$($(1)_OBJS) \
		$$($(1)_LIBS:%=-l%) $(DLL_CLIBS)
endef

define CXXMODULE_template
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -shared -Wl,-soname,$(1)$(DLL_EXT) \
		$(LDFLAGS) $(DLL_LDFLAGS) -o $(1)$(DLL_EXT) $$($(1)_OBJS) \
		$$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS)
endef

define CPROGRAM_template
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(EXE_EXT): $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) $(LDFLAGS) $(EXE_LDFLAGS) \
		-o $(1)$(EXE_EXT) $$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(EXE_CLIBS)
endef

define CXXPROGRAM_template
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(EXE_EXT): $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) $(LDFLAGS) $(EXE_LDFLAGS) \
		-o $(1)$(EXE_EXT) $$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(EXE_CXXLIBS)
endef

$(foreach x,$(CMODULES),$(eval \
	$(call CMODULE_template,$(x))))

$(foreach x,$(CXXMODULES),$(eval \
	$(call CXXMODULE_template,$(x))))

$(foreach x,$(CPROGRAMS),$(eval \
	$(call CPROGRAM_template,$(x))))

$(foreach x,$(CXXPROGRAMS),$(eval \
	$(call CXXPROGRAM_template,$(x))))

ifeq ($(UNAME), WIN32)
$(DLL_CXXCRT): /usr/lib/mingw/dllcrt2.o
	$(AR) -cr $@ $<
endif

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

-include $(DEPS)
