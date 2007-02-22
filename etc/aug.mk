.SUFFIXES:
.SUFFIXES: .c .cpp .dll .exe .so .o

SHELL = /bin/sh
UNAME := $(shell uname | sed -e 's/CYGWIN.*/WIN32/i' \
	-e 's/MINGW.*/WIN32/i')

CC = gcc
CXX = g++
RM = rm

ifeq ($(UNAME), WIN32)
DLLEXT = .dll
EXEEXT = .exe
COPTS = \
	-mno-cygwin \
	-mthreads
CDEFS = \
	-DWINVER=0x0501
else
DLLEXT = .so
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

BINS = \
	$(CLIBRARIES:%=%$(DLLEXT)) \
	$(CXXLIBRARIES:%=%$(DLLEXT)) \
	$(CMODULES:%=%$(DLLEXT)) \
	$(CXXMODULES:%=%$(DLLEXT)) \
	$(CPROGRAMS:%=%$(EXEEXT)) \
	$(CXXPROGRAMS:%=%$(EXEEXT))

.PHONY: all clean

all: $(BINS)

clean:
	$(RM) -f $(DEPS) $(BINS) $(OBJS)

define LIBRARY_template
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(DLLEXT): $$($(1)_OBJS)
	$(2) $(COPTS) $(3) -shared -Wl,-soname,$(1)$(DLLEXT) \
		$(LDFLAGS) $(CDEFS) -o $(1)$(DLLEXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%)
endef

define PROGRAM_template
DEPS += $$($(1)_OBJS:%.o=%.d)
OBJS += $$($(1)_OBJS)
$(1)$(EXEEXT): $$($(1)_OBJS)
	$(2) $(COPTS) $(3) $(LDFLAGS) $(CDEFS) -o $(1)$(EXEEXT) \
		$$($(1)_OBJS) $$($(1)_LIBS:%=-l%)
endef

$(foreach library,$(CLIBRARIES),$(eval $(call \
	LIBRARY_template,$(library),$(CC), $(CFLAGS))))

$(foreach library,$(CXXLIBRARIES),$(eval \
	$(call LIBRARY_template,$(library),$(CXX), $(CXXFLAGS))))

$(foreach module,$(CMODULES),$(eval $(call \
	LIBRARY_template,$(module),$(CC), $(CFLAGS))))

$(foreach module,$(CXXMODULES),$(eval \
	$(call LIBRARY_template,$(module),$(CXX), $(CXXFLAGS))))

$(foreach program,$(CPROGRAMS),$(eval $(call \
	PROGRAM_template,$(program),$(CC), $(CFLAGS))))

$(foreach program,$(CXXPROGRAMS),$(eval \
	$(call PROGRAM_template,$(program),$(CXX), $(CXXFLAGS))))

%.o: %.c
	$(CC) $(COPTS) $(CFLAGS) $(CDEFS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(COPTS) $(CXXFLAGS) $(CDEFS) -c -o $@ $<

-include $(DEPS)
