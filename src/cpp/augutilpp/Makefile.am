include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augutilpp
modinclude_HEADERS = \
	config.hpp \
	conv.hpp \
	file.hpp \
	log.hpp \
	path.hpp \
	signal.hpp \
	timer.hpp

if ENABLE_CPPLIBS

noinst_LTLIBRARIES = libaugutilpp.la

libaugutilpp_la_SOURCES = \
	config.cpp \
	conv.cpp \
	file.cpp \
	log.cpp \
	path.cpp \
	signal.cpp \
	timer.cpp

libaugutilpp_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugutilpp_la_LIBADD = \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augutilpp-test
TESTS = $(check_PROGRAMS)

augutilpp_test_SOURCES = test.cpp
augutilpp_test_LDADD = \
	$(top_srcdir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la \
	$(EXTRA_LIBS)

endif
