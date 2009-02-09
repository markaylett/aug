SUFFIXES = .cpp .hpp .idl

ACLOCAL_AMFLAGS = -Im4
DEFAULT_INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/c \
	-I$(top_srcdir)/src/cpp
