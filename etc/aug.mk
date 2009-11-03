.SUFFIXES:
.SUFFIXES: .c .cpp .dll .exe .so .a .o

# Standard tools.

ifndef AR
AR := ar
endif
ifndef CC
CC := gcc
endif
ifndef CXX
CXX := g++
endif
ifndef INSTALL
INSTALL := install
endif
ifndef MKDIR
MKDIR := mkdir
endif
ifndef RM
RM := rm
endif
ifndef SHELL
SHELL := sh
endif

INSTALLDIRS += bin include lib mod

# Install prefix defaults to home.

ifndef PREFIX
PREFIX = $(HOME)
endif

#### Platform Specifics ####

ifdef PLATFORM
build_ := $(shell echo $(PLATFORM) | tr '[:lower:]' '[:upper:]')
else
build_ := $(shell uname -s | sed -e 's/CYGWIN.*/CYGWIN/i' -e 's/MINGW.*/MINGW/i')
endif

ifeq ($(build_),MINGW)

win32_ := 1
copts_ = \
	-mno-cygwin \
	-mthreads
cdefs_ = \
	-DWINVER=0x0501 \
	-DFD_SETSIZE=256 \
	-D__USE_W32_SOCKETS
dll_ext_ := .dll
dll_ldflags_ = \
	-L. \
	-L/usr/lib/mingw \
	-Wl,--enable-auto-image-base
dll_clibs_ =
dll_cxxlibs_ = \
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
dll_cxxcrt_ := \
	libdllcrt2.a

exe_ext_ := .exe
exe_ldflags_ =
exe_clibs =
exe_cxxlibs_ =

else

ifeq ($(build_),CYGWIN)

win32_ := 1
copts_ =
cdefs_ =
dll_ext_ := .dll
exe_ext_ := .exe

else

# Otherwise assume Unix.

win32_ :=
copts_ = \
	-fPIC
cdefs_ = \
	-DPIC
dll_ext_ := .so
exe_ext_ :=

endif

# Common for non-MINGW.

dll_ldflags_ =
dll_clibs_ =
dll_cxxlibs_ =
dll_cxxcrt_ :=

exe_ldflags_ =
exe_clibs =
exe_cxxlibs_ =

endif

# Common for all.

cdefs_ += \
	-D_MT \
	-D_REENTRANT

copts_ += \
	-MMD \
	-MP

targets_ = \
	$(CLIBRARIES) \
	$(CXXLIBRARIES) \
	$(CMODULES) \
	$(CXXMODULES) \
	$(CPROGRAMS) \
	$(CXXPROGRAMS)

#### Function Definitions ####

bins_ :=
libs_ :=
mods_ :=

clean_ :=
objs_ :=
# defs_ left undefined due to conditional dependency inclusion.

define win32lib
bins_ += lib$(1)$(dll_ext_)
libs_ += lib$(1)$(dll_ext_).a
endef

define posixlib
libs_ += lib$(1)$(dll_ext_)
endef

#### CLIBRARY ####

define CLIBRARY_template
.PHONY: $(1)
$(1): lib$(1)$(dll_ext_)

$(eval $(1)_IMPLIB := -Wl,--out-implib,lib$(1)$(dll_ext_).a)

lib$(1)$(dll_ext_): $$($(1)_OBJS)
	$(CC) $(copts_) $(CFLAGS) $(cdefs_) \
	-shared -Wl,-soname,lib$(1)$(dll_ext_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(dll_ldflags_) \
	-o lib$(1)$(dll_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(dll_clibs_) \
	$(if $(win32_),$($(1)_IMPLIB),)

$$($(1)_OBJS): $$($(1)_DEPS)

$(if $(win32_), $(call win32lib,$(1)), $(call posixlib,$(1)))

objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXLIBRARY ####

define CXXLIBRARY_template
.PHONY: $(1)
$(1): lib$(1)$(dll_ext_)

$(eval $(1)_IMPLIB := -Wl,--out-implib,lib$(1)$(dll_ext_).a)

lib$(1)$(dll_ext_): $$($(1)_OBJS)
	$(CXX) $(copts_) $(CXXFLAGS) $(cdefs_) \
	-shared -Wl,-soname,lib$(1)$(dll_ext_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(dll_ldflags_) \
	-o lib$(1)$(dll_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(dll_cxxlibs_) \
	$(if $(win32_),$($(1)_IMPLIB),)

$$($(1)_OBJS): $(dll_cxxcrt_) $$($(1)_DEPS)

$(if $(win32_), $(call win32lib,$(1)), $(call posixlib,$(1)))

objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### CMODULE ####

define CMODULE_template
ifneq ($(1), $(1)$(dll_ext_))
.PHONY: $(1)
$(1): $(1)$(dll_ext_)
endif

$(1)$(dll_ext_): $$($(1)_OBJS)
	$(CC) $(copts_) $(CFLAGS) $(cdefs_) \
	-shared -Wl,-soname,$(1)$(dll_ext_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(dll_ldflags_) \
	-o $(1)$(dll_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(dll_clibs_)

$$($(1)_OBJS): $$($(1)_DEPS)

mods_ += $(1)$(dll_ext_)
objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXMODULE ####

define CXXMODULE_template
ifneq ($(1), $(1)$(dll_ext_))
.PHONY: $(1)
$(1): $(1)$(dll_ext_)
endif

$(1)$(dll_ext_): $$($(1)_OBJS)
	$(CXX) $(copts_) $(CXXFLAGS) $(cdefs_) \
	-shared -Wl,-soname,$(1)$(dll_ext_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(dll_ldflags_) \
	-o $(1)$(dll_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(dll_cxxlibs_)

$$($(1)_OBJS): $(dll_cxxcrt_) $$($(1)_DEPS)

mods_ += $(1)$(dll_ext_)
objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### CPROGRAM ####

define CPROGRAM_template
ifneq ($(1), $(1)$(exe_ext_))
.PHONY: $(1)
$(1): $(1)$(exe_ext_)
endif

$(1)$(exe_ext_): $$($(1)_OBJS)
	$(CC) $(copts_) $(CFLAGS) $(cdefs_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(exe_ldflags_) \
	-o $(1)$(exe_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(exe_clibs)

$$($(1)_OBJS): $$($(1)_DEPS)

bins_ += $(1)$(exe_ext_)
objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXPROGRAM ####

define CXXPROGRAM_template
ifneq ($(1), $(1)$(exe_ext_))
.PHONY: $(1)
$(1): $(1)$(exe_ext_)
endif

$(1)$(exe_ext_): $$($(1)_OBJS)
	$(CXX) $(copts_) $(CXXFLAGS) $(cdefs_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(exe_ldflags_) \
	-o $(1)$(exe_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(exe_cxxlibs_)

$$($(1)_OBJS): $$($(1)_DEPS)

bins_ += $(1)$(exe_ext_)
objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### CTEST ####

define CTEST_template
ifneq ($(1), $(1)$(exe_ext_))
.PHONY: $(1)
$(1): $(1)$(exe_ext_)
endif

$(1)$(exe_ext_): $$($(1)_OBJS)
	$(CC) $(copts_) $(CFLAGS) $(cdefs_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(exe_ldflags_) \
	-o $(1)$(exe_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(exe_clibs)

$$($(1)_OBJS): $$($(1)_DEPS)

clean_ += $(1)$(exe_ext_)
objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### CXXTEST ####

define CXXTEST_template
ifneq ($(1), $(1)$(exe_ext_))
.PHONY: $(1)
$(1): $(1)$(exe_ext_)
endif

$(1)$(exe_ext_): $$($(1)_OBJS)
	$(CXX) $(copts_) $(CXXFLAGS) $(cdefs_) \
	$(LDFLAGS) $$($(1)_LDFLAGS) $(exe_ldflags_) \
	-o $(1)$(exe_ext_) $$($(1)_OBJS) \
	$$($(1)_LIBS:%=-l%) $(exe_cxxlibs_)

$$($(1)_OBJS): $$($(1)_DEPS)

clean_ += $(1)$(exe_ext_)
objs_ += $$($(1)_OBJS)
deps_ += $$($(1)_OBJS:%.o=%.d)
endef

#### DIR ####

define DIR_template
$(1)_DIR := $(if $($(1)_DIR),$($(1)_DIR),$(PREFIX)/$(1))
endef

#### INSTALL ####

define INSTALL_template
.PHONY: install-$(1)

install-$(1): $($(1)_FILES)
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

bin_FILES += $(bins_)
lib_FILES += $(libs_)
mod_FILES += $(mods_)

# Expand install and uninstall.

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call DIR_template,$(x))))

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call INSTALL_template,$(x))))

$(foreach x,$(INSTALLDIRS),$(eval \
	$(call UNINSTALL_template,$(x))))

#### Common Targets ####

.PHONY: all clean install uninstall check

.PHONY: all-base
all-base: all-subdirs $(targets_)

.PHONY: clean-base
clean-base: clean-subdirs
	$(RM) -f $(bins_) $(libs_) $(mods_) \
	$(dll_cxxcrt_) $(clean_) $(objs_) $(deps_) *~

.PHONY: install-base
install-base: install-subdirs $(INSTALLDIRS:%=install-%)

.PHONY: uninstall-base
uninstall-base: uninstall-subdirs $(INSTALLDIRS:%=uninstall-%)

#### Unit Tests ####

.PHONY: check-base
check-base: check-subdirs $(TESTS)
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
	      exit 1; \
	  fi; \
	fi

#### Compilation Rules ####

ifeq ($(build_),MINGW)
$(dll_cxxcrt_): /usr/lib/mingw/dllcrt2.o
	$(AR) -cr $@ $<
endif

%.o: %.c
	$(CC) $(copts_) $(CFLAGS) $(cdefs_) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(copts_) $(CXXFLAGS) $(cdefs_) -c -o $@ $<

ifdef deps_
-include $(deps_)
endif
