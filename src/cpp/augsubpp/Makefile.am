include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augsubpp
modinclude_HEADERS = \
	base.hpp \
	config.hpp \
	exception.hpp \
	factory.hpp \
	inner.hpp \
	inverse.hpp \
	names.hpp \
	node.hpp \
	outer.hpp \
	query.hpp \
	recur.hpp \
	typed.hpp \
	types.hpp \
	utility.hpp \
	wild.hpp

lib_LTLIBRARIES = libaugsubpp.la

libaugsubpp_la_SOURCES = \
	base.cpp \
	config.cpp \
	exception.cpp \
	factory.cpp \
	inner.cpp \
	inverse.cpp \
	names.cpp \
	node.cpp \
	outer.cpp \
	query.cpp \
	recur.cpp \
	typed.cpp \
	types.cpp \
	utility.cpp \
	wild.cpp

libaugsubpp_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugsubpp_la_LIBADD =
