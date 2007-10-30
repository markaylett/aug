include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augmarpp
modinclude_HEADERS = \
	mar.hpp \
	compat.hpp \
	config.hpp \
	header.hpp \
	iterator.hpp \
	smartmar.hpp \
	stream.hpp \
	streambuf.hpp \
	swapfile.hpp \
	types.hpp

noinst_LTLIBRARIES = libaugmarpp.la

libaugmarpp_la_SOURCES = \
	mar.cpp \
	compat.cpp \
	config.cpp \
	header.cpp \
	iterator.cpp \
	smartmar.cpp \
	stream.cpp \
	streambuf.cpp \
	swapfile.cpp \
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
