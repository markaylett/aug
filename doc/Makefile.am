SUBDIRS = \
	man

EXTRA_DIST = \
	daug.txt \
	daugintro.mk \
	daugintro.w \
	cvs-policy.txt \
	cwebmac.tex \
	Doxyfile

dotdepend:
	for l in augsys augmar augutil augsrv augnet; do \
		(cd $(top_srcdir)/src/c && \
			../pl/dotdepend -I. `find $$l -name "*.c" | grep -v '[.]libs'`) \
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
