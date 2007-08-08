include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augutilpp
modinclude_HEADERS = \
	clock.hpp \
	config.hpp \
	conv.hpp \
	event.hpp \
	file.hpp \
	log.hpp \
	path.hpp \
	timer.hpp \
	var.hpp

noinst_LTLIBRARIES = libaugutilpp.la

libaugutilpp_la_SOURCES = \
	clock.cpp \
	config.cpp \
	conv.cpp \
	event.cpp \
	file.cpp \
	log.cpp \
	path.cpp \
	timer.cpp \
	var.cpp

libaugutilpp_la_LDFLAGS = \
	-no-undefined

libaugutilpp_la_LIBADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augutilpp-test
TESTS = augutilpp-test

augutilpp_test_SOURCES = test.cpp
augutilpp_test_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
