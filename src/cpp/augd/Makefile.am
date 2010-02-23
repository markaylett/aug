include $(top_srcdir)/src/cpp/lang.mk

noinst_HEADERS = \
	exception.hpp \
	module.hpp \
	options.hpp \
	session.hpp \
	ssl.hpp

bin_PROGRAMS = augd

augd_SOURCES = \
	augd.cpp \
	exception.cpp \
	module.cpp \
	options.cpp \
	session.cpp \
	ssl.cpp

augd_LDADD = \
	$(top_builddir)/src/cpp/augaspp/libaugaspp.la \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augserv/libaugserv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
