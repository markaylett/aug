include $(top_srcdir)/src/cpp/lang.mk

SUBDIRS = \
	test

noinst_HEADERS = \
	exception.hpp \
	module.hpp \
	options.hpp \
	session.hpp \
	ssl.hpp \
	utility.hpp

bin_PROGRAMS = daug

daug_SOURCES = \
	daug.cpp \
	exception.cpp \
	module.cpp \
	options.cpp \
	session.cpp \
	ssl.cpp \
	utility.cpp

daug_LDADD = \
	$(top_builddir)/src/cpp/augrtpp/libaugrtpp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	augrtpp-test

TESTS_ENVIRONMENT = \
	AUG_LOGLEVEL=3 \
	AUG_BUILDDIR=$(top_builddir) \
	AUG_SRCDIR=$(top_srcdir)

TESTS = \
	augrtpp-test

if ENABLE_PYTHON
TESTS += \
	test/pytest1 \
	test/pytest2 \
	test/pytest3 \
	test/pytest4 \
	test/pytest5 \
	test/pytest6 \
	test/pytest7
endif

if ENABLE_RUBY
TESTS += \
	test/rbtest1 \
	test/rbtest2 \
	test/rbtest3 \
	test/rbtest4 \
	test/rbtest5 \
	test/rbtest6 \
	test/rbtest7
endif

augrtpp_test_SOURCES = test.cpp
augrtpp_test_LDADD =
