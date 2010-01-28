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

CLEANFILES = \
	atomic.tex \
	augintro.tex \
	augintro.cpp \
	augintro.dll \
	*.dot \
	*.dvi \
	*.gif \
	*.idx \
	*.log \
	*.pdf \
	*.scn \
	*.toc

dotdepend:
	for l in augctx augsys augmar augutil augserv augnet; do \
		(cd $(top_srcdir)/src/c && \
			../pl/dotdepend $$l -I. $$(find $$l -name "*.c" \
			| egrep -v '([.]libs|test[.]c$$)')) \
			| tee $$l.dot \
			| dot -Tgif >$$l.gif; \
	done

if WITH_CWEB
BUILT_SOURCES = \
	augintro.cpp

augintro: augintro.cpp
	$(MAKE) -f augintro.mk TOP_SRCDIR=$(top_srcdir)

augintro.cpp: augintro.w
	ctangle $< - $@
	@cat notice.txt augintro.cpp | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ augintro.cpp

%.pdf: %.dvi
	dvipdfm $<

%.dvi: %.tex
	tex "\let\pdf+ \input $<"

%.tex: %.w
	cweave $<
endif
