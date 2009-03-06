include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augmarpp
modinclude_HEADERS = \
	compat.hpp \
	config.hpp \
	header.hpp \
	iterator.hpp \
	mar.hpp \
	stream.hpp \
	streambuf.hpp \
	types.hpp

noinst_LTLIBRARIES = libaugmarpp.la

libaugmarpp_la_SOURCES = \
	compat.cpp \
	config.cpp \
	header.cpp \
	iterator.cpp \
	mar.cpp \
	stream.cpp \
	streambuf.cpp \
	types.cpp

libaugmarpp_la_LDFLAGS = \
	-no-undefined

libaugmarpp_la_LIBADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = maintest
TESTS = maintest

maintest_SOURCES = maintest.cpp
maintest_LDADD = \
	$(top_builddir)/src/cpp/augmarpp/libaugmarpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la
