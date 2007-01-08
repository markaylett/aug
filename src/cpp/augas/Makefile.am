include $(top_srcdir)/src/cpp/lang.mk

EXTRA_DIST = \
	augasd.conf \
	augasd.sh

noinst_HEADERS = \
	buffer.hpp \
	conn.hpp \
	exception.hpp \
	file.hpp \
	listener.hpp \
	manager.hpp \
	module.hpp \
	options.hpp \
	sess.hpp \
	utility.hpp

bin_PROGRAMS = augasd

augasd_SOURCES = \
	augasd.cpp \
	buffer.cpp \
	conn.cpp \
	exception.cpp \
	file.cpp \
	listener.cpp \
	manager.cpp \
	module.cpp \
	options.cpp \
	sess.cpp \
	utility.cpp

augasd_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
