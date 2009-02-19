SUBDIRS = \
	man

EXTRA_DIST = \
	augserv.html \
	daugintro.mk \
	daugintro.w \
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

daugintro.pdf: daugintro.dvi
	dvipdfm $<

daugintro.dvi: daugintro.tex
	tex "\let\pdf+ \input $<"

daugintro.tex: daugintro.w
	cweave $<

daugintro: daugintro.cpp
	$(MAKE) -f daugintro.mk TOP_SRCDIR=$(top_srcdir)

daugintro.cpp: daugintro.w
	ctangle $< - $@
