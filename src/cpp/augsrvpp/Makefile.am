include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augsrvpp
modinclude_HEADERS = \
	config.hpp \
	log.hpp \
	main.hpp \
	signal.hpp

if ENABLE_CPPBUILD

noinst_LTLIBRARIES = libaugsrvpp.la

libaugsrvpp_la_SOURCES = \
	config.cpp \
	log.cpp \
	main.cpp \
	signal.cpp

libaugsrvpp_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugsrvpp_la_LIBADD = \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augsrvpp-test
TESTS = $(check_PROGRAMS)

augsrvpp_test_SOURCES = test.cpp
augsrvpp_test_LDADD = \
	$(top_srcdir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

endif
