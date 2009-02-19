SUBDIRS = \
	man

EXTRA_DIST = \
	augintro.mk \
	augintro.w \
	augserv.html \
	cvs-policy.txt \
	cwebmac.tex \
	Doxyfile

dotdepend:
	for l in augctx augsys augmar augutil augserv augnet; do \
		(cd $(top_srcdir)/src/c && \
			../pl/dotdepend $$l -I. $$(find $$l -name "*.c" \
			| egrep -v '([.]libs|test[.]c$$)')) \
			| tee $$l.dot \
			| dot -Tgif >$$l.gif; \
	done

augintro.pdf: augintro.dvi
	dvipdfm $<

augintro.dvi: augintro.tex
	tex "\let\pdf+ \input $<"

augintro.tex: augintro.w
	cweave $<

augintro: augintro.cpp
	$(MAKE) -f augintro.mk TOP_SRCDIR=$(top_srcdir)

augintro.cpp: augintro.w
	ctangle $< - $@
