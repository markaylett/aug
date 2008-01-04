.SUFFIXES:
.SUFFIXES: .c .cpp .dll .exe .so .o

SHELL = /bin/sh

ifndef PLATFORM
BUILD:=$(shell uname -s | sed -e 's/CYGWIN.*/CYGWIN/i' -e 's/MINGW.*/MINGW/i')
else
BUILD:=$(shell echo $(PLATFORM) | tr '[:lower:]' '[:upper:]')
endif

AR = ar
CC = gcc
CXX = g++
RM = rm

ifeq ($(BUILD), MINGW)
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

ifeq ($(BUILD), CYGWIN)
DLL_EXT = .dll
else
DLL_EXT = .so
endif
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

define CLIBRARY_template
TARGETS += $(1)$(DLL_EXT)
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(DLL_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -shared -Wl,-soname,$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o $(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CLIBS)
endef

define CXXLIBRARY_template
TARGETS += $(1)$(DLL_EXT)
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -shared -Wl,-soname,$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o $(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS)
endef

define CPROGRAM_template
TARGETS += $(1)$(EXE_EXT)
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) $(LDFLAGS) $$($(1)_LDFLAGS) \
		$(EXE_LDFLAGS) -o $(1)$(EXE_EXT) $$($(1)_OBJS) $$($(1)_LIBS:%=-l%) \
		$(EXE_CLIBS)
endef

define CXXPROGRAM_template
TARGETS += $(1)$(EXE_EXT)
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) $(LDFLAGS) $$($(1)_LDFLAGS) \
		$(EXE_LDFLAGS) -o $(1)$(EXE_EXT) $$($(1)_OBJS) $$($(1)_LIBS:%=-l%) \
		$(EXE_CXXLIBS)
endef

$(foreach x,$(CLIBRARIES),$(eval \
	$(call CLIBRARY_template,$(x))))

$(foreach x,$(CXXLIBRARIES),$(eval \
	$(call CXXLIBRARY_template,$(x))))

$(foreach x,$(CMODULES),$(eval \
	$(call CLIBRARY_template,$(x))))

$(foreach x,$(CXXMODULES),$(eval \
	$(call CXXLIBRARY_template,$(x))))

$(foreach x,$(CPROGRAMS),$(eval \
	$(call CPROGRAM_template,$(x))))

$(foreach x,$(CXXPROGRAMS),$(eval \
	$(call CXXPROGRAM_template,$(x))))

.PHONY: all all-aug clean clean-aug

all-aug: $(TARGETS)

clean-aug:
	$(RM) -f $(DEPS) $(TARGETS) $(DLL_CXXCRT) $(OBJS) *~

ifeq ($(BUILD), MINGW)
$(DLL_CXXCRT): /usr/lib/mingw/dllcrt2.o
	$(AR) -cr $@ $<
endif

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

-include $(DEPS)
