include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augservpp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	log.hpp \
	main.hpp \
	signal.hpp

noinst_LTLIBRARIES = libaugservpp.la

libaugservpp_la_SOURCES = \
	base.cpp \
	config.cpp \
	log.cpp \
	main.cpp \
	signal.cpp

libaugservpp_la_LDFLAGS = \
	-no-undefined

libaugservpp_la_LIBADD = \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augserv/libaugserv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = maintest
TESTS = maintest

maintest_SOURCES = maintest.cpp
maintest_LDADD = \
	$(top_builddir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_builddir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augserv/libaugserv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
