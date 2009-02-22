include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augsyspp
modinclude_HEADERS = \
	config.hpp \
	dlfcn.hpp \
	endpoint.hpp \
	inetaddr.hpp \
	mcast.hpp \
	mmap.hpp \
	muxer.hpp \
	smartfd.hpp \
	smartptr.hpp \
	socket.hpp \
	time.hpp \
	types.hpp \
	unistd.hpp \
	utility.hpp

noinst_LTLIBRARIES = libaugsyspp.la

libaugsyspp_la_SOURCES = \
	config.cpp \
	dlfcn.cpp \
	endpoint.cpp \
	inetaddr.cpp \
	mcast.cpp \
	mmap.cpp \
	muxer.cpp \
	smartfd.cpp \
	smartptr.cpp \
	socket.cpp \
	time.cpp \
	types.cpp \
	unistd.cpp \
	utility.cpp

libaugsyspp_la_LDFLAGS = \
	-no-undefined

libaugsyspp_la_LIBADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	muxertest \
	smartptrtest

TESTS = \
	muxertest \
	smartptrtest

muxertest_SOURCES = muxertest.cpp
muxertest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

smartptrtest_SOURCES = smartptrtest.cpp
smartptrtest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
