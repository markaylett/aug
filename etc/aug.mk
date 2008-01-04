.SUFFIXES:
.SUFFIXES: .c .cpp .dll .exe .so .a .o

SHELL = /bin/sh

ifndef PLATFORM
BUILD:=$(shell uname -s | sed -e 's/CYGWIN.*/CYGWIN/i' -e 's/MINGW.*/MINGW/i')
else
BUILD:=$(shell echo $(PLATFORM) | tr '[:lower:]' '[:upper:]')
endif

ifndef PREFIX
PREFIX = $(HOME)
endif

AR = ar
CC = gcc
CXX = g++
INSTALL = install
RM = rm

ifeq ($(BUILD), MINGW)
COPTS = \
	-mno-cygwin \
	-mthreads
CDEFS = \
	-DWINVER=0x0501 \
	-DFD_SETSIZE=256 \
	-D__USE_W32_SOCKETS
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

ifeq ($(BUILD), CYGWIN)
COPTS =
CDEFS =
DLL_EXT = .dll
EXE_EXT = .exe
else
BUILD = UNIX
COPTS = \
	-fPIC
CDEFS = \
	-DPIC
DLL_EXT = .so
EXE_EXT =
endif
DLL_LDFLAGS =
DLL_CLIBS =
DLL_CXXLIBS =
DLL_CXXCRT =

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
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
$(1): lib$(1)$(DLL_EXT)

ifneq ($(BUILD), UNIX)
BINS += lib$(1)$(DLL_EXT)
LIBS += lib$(1)$(DLL_EXT).a
lib$(1)$(DLL_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -shared -Wl,-soname,lib$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o lib$(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CLIBS) \
		-Wl,--out-implib,lib$(1)$(DLL_EXT).a
else
LIBS += lib$(1)$(DLL_EXT)
lib$(1)$(DLL_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -shared -Wl,-soname,lib$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o lib$(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CLIBS)
endif
endef

define CXXLIBRARY_template
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
$(1): lib$(1)$(DLL_EXT)

ifneq ($(BUILD), UNIX)
BINS += lib$(1)$(DLL_EXT)
LIBS += lib$(1)$(DLL_EXT).a
lib$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -shared -Wl,-soname,lib$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o lib$(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS) \
		-Wl,--out-implib,lib$(1)$(DLL_EXT).a
else
LIBS += lib$(1)$(DLL_EXT)
lib$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -shared -Wl,-soname,lib$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o lib$(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS)
endif
endef

define CMODULE_template
MODS += $(1)$(DLL_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
ifneq ($(1), $(1)$(DLL_EXT))
$(1): $(1)$(DLL_EXT)
endif

$(1)$(DLL_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -shared -Wl,-soname,$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o $(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CLIBS)
endef

define CXXMODULE_template
MODS += $(1)$(DLL_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
ifneq ($(1), $(1)$(DLL_EXT))
$(1): $(1)$(DLL_EXT)
endif

$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -shared -Wl,-soname,$(1)$(DLL_EXT) \
		$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) -o $(1)$(DLL_EXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS)
endef

define CPROGRAM_template
BINS += $(1)$(EXE_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
ifneq ($(1), $(1)$(EXE_EXT))
$(1): $(1)$(EXE_EXT)
endif

$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) $(LDFLAGS) $$($(1)_LDFLAGS) \
		$(EXE_LDFLAGS) -o $(1)$(EXE_EXT) $$($(1)_OBJS) $$($(1)_LIBS:%=-l%) \
		$(EXE_CLIBS)
endef

define CXXPROGRAM_template
BINS += $(1)$(EXE_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
ifneq ($(1), $(1)$(EXE_EXT))
$(1): $(1)$(EXE_EXT)
endif

$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) $(LDFLAGS) $$($(1)_LDFLAGS) \
		$(EXE_LDFLAGS) -o $(1)$(EXE_EXT) $$($(1)_OBJS) $$($(1)_LIBS:%=-l%) \
		$(EXE_CXXLIBS)
endef

define INSTALL_template
ifndef $(1)_DIR
install-$(1): all-aug
	@mkdir -p $(PREFIX)/$(1)
	@for f in $($(1)_FILES); do $(INSTALL) -pv $$$$f $(PREFIX)/$(1); done
else
install-$(1): all-aug
	@mkdir -p $($(1)_DIR)
	@for f in $($(1)_FILES); do $(INSTALL) -pv $$$$f $($(1)_DIR); done
endif
endef

define UNINSTALL_template
ifndef $(1)_DIR
uninstall-$(1):
	@for f in $($(1)_FILES); do $(RM) -fv $(PREFIX)/$(1)/$$$$f; done
else
uninstall-$(1):
	@for f in $($(1)_FILES); do $(RM) -fv $($(1)_DIR)/$$$$f; done
endif
endef

$(foreach x,$(CLIBRARIES),$(eval \
	$(call CLIBRARY_template,$(x))))

$(foreach x,$(CXXLIBRARIES),$(eval \
	$(call CXXLIBRARY_template,$(x))))

$(foreach x,$(CMODULES),$(eval \
	$(call CMODULE_template,$(x))))

$(foreach x,$(CXXMODULES),$(eval \
	$(call CXXMODULE_template,$(x))))

$(foreach x,$(CPROGRAMS),$(eval \
	$(call CPROGRAM_template,$(x))))

$(foreach x,$(CXXPROGRAMS),$(eval \
	$(call CXXPROGRAM_template,$(x))))

INSTALLDIRS += bin include lib mod
bin_FILES += $(BINS)
lib_FILES += $(LIBS)
mod_FILES += $(MODS)

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call INSTALL_template,$(x))))

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call UNINSTALL_template,$(x))))

TARGETS = \
	$(CLIBRARIES) \
	$(CXXLIBRARIES) \
	$(CMODULES) \
	$(CXXMODULES) \
	$(CPROGRAMS) \
	$(CXXPROGRAMS)

.PHONY: all all-aug clean clean-aug install install-aug $(TARGETS)

all-aug: $(TARGETS)

clean-aug:
	$(RM) -f $(BINS) $(LIBS) $(MODS) $(DLL_CXXCRT) $(OBJS) $(DEPS) *~

install-aug: $(INSTALLDIRS:%=install-%)

uninstall-aug: $(INSTALLDIRS:%=uninstall-%)


ifeq ($(BUILD), MINGW)
$(DLL_CXXCRT): /usr/lib/mingw/dllcrt2.o
	$(AR) -cr $@ $<
endif

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

-include $(DEPS)
