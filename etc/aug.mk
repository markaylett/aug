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

ifeq ($(BUILD),MINGW)

WIN32 := 1
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

ifeq ($(BUILD),CYGWIN)

WIN32 := 1
COPTS =
CDEFS =
DLL_EXT := .dll
EXE_EXT := .exe

else

# Otherwise assume Unix.

WIN32 :=
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

TARGETS = \
	$(CLIBRARIES) \
	$(CXXLIBRARIES) \
	$(CMODULES) \
	$(CXXMODULES) \
	$(CPROGRAMS) \
	$(CXXPROGRAMS)

#### Function Definitions ####

BINS :=
LIBS :=
MODS :=

CLEAN :=
OBJS :=

define win32lib
BINS += lib$(1)$(DLL_EXT)
LIBS += lib$(1)$(DLL_EXT).a
endef

define posixlib
LIBS += lib$(1)$(DLL_EXT)
endef

#### CLIBRARY ####

define CLIBRARY_template
.PHONY: $(1)
$(1): lib$(1)$(DLL_EXT)

$(eval $(1)_IMPLIB := -Wl,--out-implib,lib$(1)$(DLL_EXT).a)

lib$(1)$(DLL_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) \
	-shared -Wl,-soname,lib$(1)$(DLL_EXT) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) \
	-o lib$(1)$(DLL_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(DLL_CLIBS) \
	$(if $(WIN32),$($(1)_IMPLIB),)

$(if $(WIN32), $(call win32lib,$(1)), $(call posixlib,$(1)))

OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXLIBRARY ####

define CXXLIBRARY_template
.PHONY: $(1)
$(1): lib$(1)$(DLL_EXT)

$(eval $(1)_IMPLIB := -Wl,--out-implib,lib$(1)$(DLL_EXT).a)

lib$(1)$(DLL_EXT): $(DLL_CXXCRT) $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) \
	-shared -Wl,-soname,lib$(1)$(DLL_EXT) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(DLL_LDFLAGS) \
	-o lib$(1)$(DLL_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(DLL_CXXLIBS) \
	$(if $(WIN32),$($(1)_IMPLIB),)

$(if $(WIN32), $(call win32lib,$(1)), $(call posixlib,$(1)))

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

#### CTEST ####

define CTEST_template
ifneq ($(1), $(1)$(EXE_EXT))
.PHONY: $(1)
$(1): $(1)$(EXE_EXT)
endif

$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(EXE_LDFLAGS) \
	-o $(1)$(EXE_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(EXE_CLIBS)

CLEAN += $(1)$(EXE_EXT)
OBJS += $$($(1)_OBJS)
DEPS += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXTEST ####

define CXXTEST_template
ifneq ($(1), $(1)$(EXE_EXT))
.PHONY: $(1)
$(1): $(1)$(EXE_EXT)
endif

$(1)$(EXE_EXT): $$($(1)_DEPS) $$($(1)_OBJS)
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(EXE_LDFLAGS) \
	-o $(1)$(EXE_EXT) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(EXE_CXXLIBS)

CLEAN += $(1)$(EXE_EXT)
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

install-$(1): $(TARGETS)
	@$(MKDIR) -p $($(1)_DIR)
	@for f in $($(1)_INSTALL); do \
		$(INSTALL) -pv $$$$f $($(1)_DIR); \
	done
endef

#### UNINSTALL ####

define UNINSTALL_template
.PHONY: uninstall-$(1)

uninstall-$(1):
	@for f in $($(1)_INSTALL); do \
		$(RM) -fv $($(1)_DIR)/$$$$f; \
	done
endef

#### SUBDIRS ####

define SUBMAKE_template
.PHONY: $(1)-$(2)
$(1)-$(2): $$($(2)_DEPS:%=$(1)-%)
	$(MAKE) -C $(2) $(1)
endef

define TARGET_template
$(foreach x,$(SUBDIRS),$(eval \
	$(call SUBMAKE_template,$(1),$(x))))

.PHONY: $(1)-subdirs
$(1)-subdirs: $$(SUBDIRS:%=$(1)-%)
endef

define SUBDIR_template
.PHONY: $(1)
$(1): all-$(1)
endef

#### Template Expansions ####

$(foreach x,all clean install uninstall check,$(eval \
	$(call TARGET_template,$(x))))

$(foreach x,$(SUBDIRS),$(eval \
	$(call SUBDIR_template,$(x))))

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

$(foreach x,$(CTESTS),$(eval \
	$(call CTEST_template,$(x))))

$(foreach x,$(CXXTESTS),$(eval \
	$(call CXXTEST_template,$(x))))

# Add installable files.

bin_INSTALL += $(BINS)
lib_INSTALL += $(LIBS)
mod_INSTALL += $(MODS)

# Expand install and uninstall.

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call DIR_template,$(x))))

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call INSTALL_template,$(x))))

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call UNINSTALL_template,$(x))))

#### Common Targets ####

.PHONY: all clean install uninstall check

.PHONY: all-aug
all-aug: all-subdirs $(TARGETS)

.PHONY: clean-aug
clean-aug: clean-subdirs
	$(RM) -f $(BINS) $(LIBS) $(MODS) \
	$(DLL_CXXCRT) $(CLEAN) $(OBJS) $(DEPS) *~

.PHONY: install-aug
install-aug: install-subdirs $(INSTALLDIRS:%=install-%)

.PHONY: uninstall-aug
uninstall-aug: uninstall-subdirs $(INSTALLDIRS:%=uninstall-%)

#### Unit Tests ####

.PHONY: check-aug
check-aug: check-subdirs $(TESTS)
	@all=0; failed=0; \
	list='$(TESTS)'; \
	if test -n "$$list"; then \
	  for t in $$list; do \
	    if test -f ./$$t; then dir=./; \
	    else dir=; fi; \
	    if $${dir}$$t; then \
	      echo "PASS: $$t"; \
	    else \
	      echo "FAIL: $$t"; \
	      failed=`expr $$failed + 1`; \
	    fi; \
	    all=`expr $$all + 1`; \
	  done; \
	  if test "$$failed" -eq 0; then \
	      echo "All $$all tests passed"; \
	  else \
	      echo "$$failed of $$all tests failed"; \
	  fi; \
	fi

#### Compilation Rules ####

ifeq ($(BUILD),MINGW)
$(DLL_CXXCRT): /usr/lib/mingw/dllcrt2.o
	$(AR) -cr $@ $<
endif

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

ifdef DEPS
-include $(DEPS)
endif
