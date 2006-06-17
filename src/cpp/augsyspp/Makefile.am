include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augsyspp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	exception.hpp \
	mplexer.hpp \
	null.hpp \
	smartfd.hpp \
	socket.hpp \
	string.hpp \
	types.hpp \
	unistd.hpp \
	utility.hpp

if ENABLE_CPPBUILD

noinst_LTLIBRARIES = libaugsyspp.la

libaugsyspp_la_SOURCES = \
	base.cpp \
	config.cpp \
	exception.cpp \
	mplexer.cpp \
	null.cpp \
	smartfd.cpp \
	socket.cpp \
	string.cpp \
	types.cpp \
	unistd.cpp \
	utility.cpp

libaugsyspp_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugsyspp_la_LIBADD = \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augsyspp-test
TESTS = $(check_PROGRAMS)

augsyspp_test_SOURCES = test.cpp
augsyspp_test_LDADD = \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

endif
