include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augutilpp
modinclude_HEADERS = \
	channels.hpp \
	clock.hpp \
	config.hpp \
	conv.hpp \
	event.hpp \
	file.hpp \
	lexer.hpp \
	log.hpp \
	object.hpp \
	path.hpp \
	shellparser.hpp \
	timer.hpp

noinst_LTLIBRARIES = libaugutilpp.la

libaugutilpp_la_SOURCES = \
	channels.cpp \
	clock.cpp \
	config.cpp \
	conv.cpp \
	event.cpp \
	file.cpp \
	lexer.cpp \
	log.cpp \
	object.cpp \
	path.cpp \
	shellparser.cpp \
	timer.cpp

libaugutilpp_la_LDFLAGS = \
	-no-undefined

libaugutilpp_la_LIBADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	maintest \
	shellparsertest

TESTS = \
	maintest \
	shellparsertest

maintest_SOURCES = maintest.cpp
maintest_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

shellparsertest_SOURCES = shellparsertest.cpp
shellparsertest_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
