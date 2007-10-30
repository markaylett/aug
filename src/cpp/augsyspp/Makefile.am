include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augsyspp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	dlfcn.hpp \
	endpoint.hpp \
	errinfo.hpp \
	exception.hpp \
	inetaddr.hpp \
	lock.hpp \
	mcast.hpp \
	mmap.hpp \
	mutex.hpp \
	muxer.hpp \
	null.hpp \
	smartfd.hpp \
	smartptr.hpp \
	socket.hpp \
	string.hpp \
	time.hpp \
	types.hpp \
	unistd.hpp \
	utility.hpp

noinst_LTLIBRARIES = libaugsyspp.la

libaugsyspp_la_SOURCES = \
	base.cpp \
	config.cpp \
	dlfcn.cpp \
	endpoint.cpp \
	errinfo.cpp \
	exception.cpp \
	inetaddr.cpp \
	lock.cpp \
	mcast.cpp \
	mmap.cpp \
	mutex.cpp \
	muxer.cpp \
	null.cpp \
	smartfd.cpp \
	smartptr.cpp \
	socket.cpp \
	string.cpp \
	time.cpp \
	types.cpp \
	unistd.cpp \
	utility.cpp

libaugsyspp_la_LDFLAGS = \
	-no-undefined

libaugsyspp_la_LIBADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	maintest \
	exceptiontest \
	mplexertest \
	smartptrtest

TESTS = \
	maintest \
	exceptiontest \
	mplexertest \
	smartptrtest

maintest_SOURCES = maintest.cpp
maintest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

exceptiontest_SOURCES = exceptiontest.cpp
exceptiontest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

mplexertest_SOURCES = mplexertest.cpp
mplexertest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

smartptrtest_SOURCES = smartptrtest.cpp
smartptrtest_LDADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
