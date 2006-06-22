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

if ENABLE_CPPBUILD

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
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

check_PROGRAMS = augmarpp-test
TESTS = augmarpp-test

augmarpp_test_SOURCES = test.cpp
augmarpp_test_LDADD = \
	$(top_srcdir)/src/cpp/augmarpp/libaugmarpp.la \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la

endif
