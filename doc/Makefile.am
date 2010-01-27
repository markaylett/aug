include $(top_srcdir)/src/cpp/lang.mk

SUBDIRS = \
	man

EXTRA_DIST = \
	atomic.w \
	augintro.mk \
	augintro.w \
	augserv.html \
	cvs-policy.txt \
	cwebmac.tex \
	notice.txt \
	Doxyfile

BUILT_SOURCES = \
	atomic.h \
	atomic.c \
	test.c \
	augintro.cpp

CLEANFILES = \
	*.c \
	*.cpp \
	*.dll \
	*.dot \
	*.dvi \
	*.gif \
	*.h \
	*.idx \
	*.log \
	*.pdf \
	*.scn \
	*.toc

check_PROGRAMS = \
	test

TESTS = \
	test

test_SOURCES = test.c

dotdepend:
	for l in augctx augsys augmar augutil augserv augnet; do \
		(cd $(top_srcdir)/src/c && \
			../pl/dotdepend $$l -I. $$(find $$l -name "*.c" \
			| egrep -v '([.]libs|test[.]c$$)')) \
			| tee $$l.dot \
			| dot -Tgif >$$l.gif; \
	done

augintro: augintro.cpp
	$(MAKE) -f augintro.mk TOP_SRCDIR=$(top_srcdir)

atomic.h atomic.c test.c: atomic.w
	ctangle $< - test.c
	@cat notice.txt atomic.h | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ atomic.h
	@cat notice.txt atomic.c | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ atomic.c
	@cat notice.txt test.c | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ test.c

augintro.cpp: augintro.w
	ctangle $< - $@
	@cat notice.txt augintro.cpp | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ augintro.cpp

%.pdf: %.dvi
	dvipdfm $<

%.dvi: %.tex
	tex "\let\pdf+ \input $<"

%.tex: %.w
	cweave $<
