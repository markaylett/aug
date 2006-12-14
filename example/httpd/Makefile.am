include $(top_srcdir)/src/c/lang.mk

noinst_PROGRAMS = httpd

httpd_SOURCES = \
	message.c \
	message.h \
	httpd.c \
	state.c \
	state.h

httpd_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la
