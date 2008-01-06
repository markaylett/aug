.SUFFIXES:
.SUFFIXES: .c .cpp .dll .exe .so .a .o

# Standard tools.

AR := ar
CC := gcc
CXX := g++
INSTALL := install
MKDIR := mkdir
RM := rm
SHELL := /bin/sh

INSTALLDIRS += bin include lib mod

# Install prefix defaults to home.

ifndef PREFIX
PREFIX = $(HOME)
endif

#### Platform Specifics ####

ifdef PLATFORM
BUILD := $(shell echo $(PLATFORM) | tr '[:lower:]' '[:upper:]')
else
BUILD := $(shell uname -s | sed -e 's/CYGWIN.*/CYGWIN/i' -e 's/MINGW.*/MINGW/i')
endif

ifeq ($(BUILD), MINGW)
COPTS = \
	-mno-cygwin \
	-mthreads
CDEFS = \
	-DWINVER=0x0501 \
	-DFD_SETSIZE=256 \
	-D__USE_W32_SOCKETS
DLL_EXT := .dll
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
DLL_CXXCRT := \
	libdllcrt2.a

EXE_EXT := .exe
EXE_LDFLAGS =
EXE_CLIBS =
EXE_CXXLIBS =

else

ifeq ($(BUILD), CYGWIN)
COPTS =
CDEFS =
DLL_EXT := .dll
EXE_EXT := .exe

else

# Otherwise assume Unix.

BUILD := UNIX
COPTS = \
	-fPIC
CDEFS = \
	-DPIC
DLL_EXT := .so
EXE_EXT :=

endif

# Common for non-MINGW.

DLL_LDFLAGS =
DLL_CLIBS =
DLL_CXXLIBS =
DLL_CXXCRT :=

EXE_LDFLAGS =
EXE_CLIBS =
EXE_CXXLIBS =

endif

# Common for all.

CDEFS += \
	-D_MT \
	-D_REENTRANT

COPTS += \
	-MMD \
	-MP

#### Template Definitions ####

TARGETS = \
	$(CLIBRARIES) \
	$(CXXLIBRARIES) \
	$(CMODULES) \
	$(CXXMODULES) \
	$(CPROGRAMS) \
	$(CXXPROGRAMS)

#### CLIBRARY ####

define CLIBRARY_template
.PHONY: $(1)
$(1): lib$(1)$(DLL_EXT)

ifeq ($(BUILD), UNIX)
$(eval $(1)_IMPLIB :=)
else
$(eval $(1)_IMPLIB := -Wl,--out-implib,lib$(1)$(DLL_EXT).a)
endif

lib$(1)$(DLL_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) \
	-shared -Wl,-soname,lib$(1)$(DLL_EXT) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) \
	-o lib$(1)$(DLL_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(DLL_CLIBS) \
	$($(1)_IMPLIB)

ifeq ($(BUILD), UNIX)
LIBS += lib$(1)$(DLL_EXT)
else
BINS += lib$(1)$(DLL_EXT)
LIBS += lib$(1)$(DLL_EXT).a
endif
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXLIBRARY ####

define CXXLIBRARY_template
.PHONY: $(1)
$(1): lib$(1)$(DLL_EXT)

ifeq ($(BUILD), UNIX)
$(eval $(1)_IMPLIB :=)
else
$(eval $(1)_IMPLIB := -Wl,--out-implib,lib$(1)$(DLL_EXT).a)
endif

lib$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) \
	-shared -Wl,-soname,lib$(1)$(DLL_EXT) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) \
	-o lib$(1)$(DLL_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS) \
	$($(1)_IMPLIB)

ifeq ($(BUILD), UNIX)
LIBS += lib$(1)$(DLL_EXT)
else
BINS += lib$(1)$(DLL_EXT)
LIBS += lib$(1)$(DLL_EXT).a
endif
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### CMODULE ####

define CMODULE_template
ifneq ($(1), $(1)$(DLL_EXT))
.PHONY: $(1)
$(1): $(1)$(DLL_EXT)
endif

$(1)$(DLL_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) \
	-shared -Wl,-soname,$(1)$(DLL_EXT) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) \
	-o $(1)$(DLL_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(DLL_CLIBS)

MODS += $(1)$(DLL_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXMODULE ####

define CXXMODULE_template
ifneq ($(1), $(1)$(DLL_EXT))
.PHONY: $(1)
$(1): $(1)$(DLL_EXT)
endif

$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) \
	-shared -Wl,-soname,$(1)$(DLL_EXT) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) \
	-o $(1)$(DLL_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS)

MODS += $(1)$(DLL_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### CPROGRAM ####

define CPROGRAM_template
ifneq ($(1), $(1)$(EXE_EXT))
.PHONY: $(1)
$(1): $(1)$(EXE_EXT)
endif

$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(EXE_LDFLAGS) \
	-o $(1)$(EXE_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(EXE_CLIBS)

BINS += $(1)$(EXE_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXPROGRAM ####

define CXXPROGRAM_template
ifneq ($(1), $(1)$(EXE_EXT))
.PHONY: $(1)
$(1): $(1)$(EXE_EXT)
endif

$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(EXE_LDFLAGS) \
	-o $(1)$(EXE_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(EXE_CXXLIBS)

BINS += $(1)$(EXE_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### DIR ####

define DIR_template
$(1)_DIR := $(if $($(1)_DIR),$($(1)_DIR),$(PREFIX)/$(1))
endef

#### INSTALL ####

define INSTALL_template
.PHONY: install-$(1)

install-$(1): all-aug
	@$(MKDIR) -p $($(1)_DIR)
	@for f in $($(1)_FILES); do \
		$(INSTALL) -pv $$$$f $($(1)_DIR); \
	done
endef

#### UNINSTALL ####

define UNINSTALL_template
.PHONY: uninstall-$(1)

uninstall-$(1):
	@for f in $($(1)_FILES); do \
		$(RM) -fv $($(1)_DIR)/$$$$f; \
	done
endef

#### Template Expansions ####

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

# Add installable files.

bin_FILES += $(BINS)
lib_FILES += $(LIBS)
mod_FILES += $(MODS)

# Expand install and uninstall.

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call DIR_template,$(x))))

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call INSTALL_template,$(x))))

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call UNINSTALL_template,$(x))))

#### Common Targets ####

.PHONY: all clean install uninstall

.PHONY: all-aug
all-aug: $(TARGETS)

.PHONY: clean-aug
clean-aug:
	$(RM) -f $(BINS) $(LIBS) $(MODS) $(DLL_CXXCRT) $(OBJS) $(DEPS) *~

.PHONY: install-aug
install-aug: $(INSTALLDIRS:%=install-%)

.PHONY: uninstall-aug
uninstall-aug: $(INSTALLDIRS:%=uninstall-%)

#### Compilation Rules ####

ifeq ($(BUILD), MINGW)
$(DLL_CXXCRT): /usr/lib/mingw/dllcrt2.o
	$(AR) -cr $@ $<
endif

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

-include $(DEPS)
