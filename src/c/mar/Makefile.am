include $(top_srcdir)/src/c/lang.mk

bin_PROGRAMS = mar

mar_SOURCES = \
	config.c \
	config.h \
	getopt.c \
	getopt.h \
	mar.c \
	utility.c \
	utility.h

mar_LDADD = \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(WINSOCK) \
	-lm
